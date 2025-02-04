//
// Created by Nikita on 20.11.2020.
//

#include <Utils/ResourceManager/ResourceManager.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/Common/Features.h>
#include <Utils/Common/Vertices.hpp>

#include <Graphics/Types/Skybox.h>
#include <Graphics/Render/Render.h>
#include <Graphics/Types/Vertices.h>
#include <Graphics/Loaders/ObjLoader.h>

#include <stbi/stb_image.h>

namespace SR_GTYPES_NS {
    Skybox::Skybox()
        : IResource(SR_COMPILE_TIME_CRC32_TYPE_NAME(Skybox), true /** auto remove */)
    { }

    Skybox::~Skybox() {
        SetShader(nullptr);

        SRAssert(
            m_cubeMap == SR_ID_INVALID &&
            m_virtualUBO == SR_ID_INVALID &&
            m_VBO == SR_ID_INVALID &&
            m_IBO == SR_ID_INVALID &&
            m_VAO == SR_ID_INVALID
        );

        for (auto&& img : m_data) {
            if (!img) {
                continue;
            }

            stbi_image_free(img);
            img = nullptr;
        }
    }

    Skybox *Skybox::Load(const SR_UTILS_NS::Path& rawPath) {
        SR_GLOBAL_LOCK

        SR_UTILS_NS::Path&& path = SR_UTILS_NS::Path(rawPath).RemoveSubPath(SR_UTILS_NS::ResourceManager::Instance().GetResPath());

        if (auto&& pResource = SR_UTILS_NS::ResourceManager::Instance().Find<Skybox>(path)) {
            return pResource;
        }

        auto&& folder = SR_UTILS_NS::ResourceManager::Instance().GetResPath().Concat(path.GetWithoutExtension());

        SR_LOG("Skybox::Load() : loading \"" + path.ToString() + "\" skybox...");

        std::array<uint8_t *, 6> sides = {};

        static constexpr const char* files[6] { "right", "left", "top", "bottom", "front", "back" };

        int32_t W, H, C;

        for (uint8_t i = 0; i < 6; ++i) {
            auto&& file = folder.Concat(files[i]).ConcatExt(path.GetExtension());

            int32_t w = 0, h = 0, comp = 0;
            uint8_t *data = stbi_load(file.CStr(), &w, &h, &comp, STBI_rgb_alpha);

            if (!data) {
                SR_ERROR("Skybox::Load() : failed to load skybox!\n\tPath: " + file.ToString());
                return nullptr;
            }

            if (i == 0) {
                W = w;
                H = h;
                C = comp;
            }
            else if (h != H || w != W || C != comp) {
                SR_WARN("Skybox::Load() : \"" + path.ToString() + "\" skybox has different sizes!");
            }

            sides[i] = data;
        }

        auto pSkybox = new Skybox();

        pSkybox->m_width = W;
        pSkybox->m_height = H;
        pSkybox->m_data = sides;

        pSkybox->SetId(path.ToString());

        return pSkybox;
    }

    bool Skybox::Calculate() {
        if (m_isCalculated) {
            SR_ERROR("Skybox::Calculate() : the skybox is already calculated!");
            return false;
        }

        const bool cpuUsage = SR_UTILS_NS::Features::Instance().Enabled("SkyboxCPUUsage", false);
        if (m_cubeMap = m_pipeline->CalculateCubeMap(m_width, m_height, m_data, cpuUsage); m_cubeMap < 0) {
            SR_ERROR("Skybox::Calculate() : failed to calculate cube map!");
            m_hasErrors = true;
            return false;
        }

        //for (auto&& img : m_data) {
        //    if (!img) {
        //        continue;
        //    }
        //
        //    stbi_image_free(img);
        //    img = nullptr;
        //}

        auto &&indexedVertices = Vertices::CastVertices<Vertices::SimpleVertex>(SR_UTILS_NS::SKYBOX_INDEXED_VERTICES);

        if (m_pipeline->GetType() == PipelineType::Vulkan) {
            auto &&indices = SR_UTILS_NS::SKYBOX_INDICES;

            if (m_VBO = m_pipeline->CalculateVBO(indexedVertices.data(), Vertices::VertexType::SimpleVertex, indexedVertices.size()); m_VBO == SR_ID_INVALID) {
                SR_ERROR("Skybox::Calculate() : failed to calculate VBO!");
                m_hasErrors = true;
                return false;
            }

            if (m_IBO = m_pipeline->CalculateIBO((void *) indices.data(), sizeof(uint32_t), indices.size(), SR_ID_INVALID);
                    m_IBO == SR_ID_INVALID) {
                SR_ERROR("Skybox::Calculate() : failed to calculate IBO!");
                m_hasErrors = true;
                return false;
            }
        }
        else {
            auto &&vertices = SR_UTILS_NS::IndexedVerticesToNonIndexed(indexedVertices, SR_UTILS_NS::SKYBOX_INDICES);

            if (m_VBO = m_pipeline->CalculateVBO(vertices.data(), Vertices::VertexType::SimpleVertex, vertices.size()); m_VBO == SR_ID_INVALID) {
                SR_ERROR("Skybox::Calculate() : failed to calculate VBO!");
                m_hasErrors = true;
                return false;
            }
        }

        m_isCalculated = true;

        return true;
    }

    void Skybox::DrawOpenGL() {

    }

    void Skybox::DrawVulkan() {
        auto&& uboManager = Memory::UBOManager::Instance();

        if (m_dirtyShader)
        {
            m_dirtyShader = false;

            m_virtualUBO = uboManager.ReAllocateUBO(m_virtualUBO, m_shader->GetUBOBlockSize(), m_shader->GetSamplersCount());

            if (m_virtualUBO != SR_ID_INVALID) {
                uboManager.BindUBO(m_virtualUBO);
            }
            else {
                m_pipeline->ResetDescriptorSet();
                m_hasErrors = true;
                return;
            }

            m_shader->InitUBOBlock();
            m_shader->Flush();

            m_shader->SetSamplerCube(SHADER_SKYBOX_DIFFUSE, m_cubeMap);
        }

        m_pipeline->BindVBO(m_VBO);
        m_pipeline->BindIBO(m_IBO);

        switch (uboManager.BindUBO(m_virtualUBO)) {
            case Memory::UBOManager::BindResult::Duplicated:
                m_shader->InitUBOBlock();
                m_shader->Flush();
                m_shader->SetSamplerCube(SHADER_SKYBOX_DIFFUSE, m_cubeMap);
                SR_FALLTHROUGH;
            case Memory::UBOManager::BindResult::Success:
                m_pipeline->DrawIndices(36);
                break;
            case Memory::UBOManager::BindResult::Failed:
            default:
                break;
        }
    }

    void Skybox::FreeVideoMemory() {
        if (!m_isCalculated) {
            return;
        }

        SR_LOG("Skybox::FreeVideoMemory() : free skybox video memory...");

        if (m_VAO != SR_ID_INVALID && !m_pipeline->FreeVAO(&m_VAO)) {
            SR_ERROR("Skybox::FreeVideoMemory() : failed to free VAO!");
        }

        if (m_VBO != SR_ID_INVALID && !m_pipeline->FreeVBO(&m_VBO)) {
            SR_ERROR("Skybox::FreeVideoMemory() : failed to free VBO!");
        }

        if (m_IBO != SR_ID_INVALID && !m_pipeline->FreeIBO(&m_IBO)) {
            SR_ERROR("Skybox::FreeVideoMemory() : failed to free IBO!");
        }

        if (m_cubeMap != SR_ID_INVALID && !m_pipeline->FreeCubeMap(&m_cubeMap)) {
            SR_ERROR("Skybox::FreeVideoMemory() : failed to free cube map!");
        }

        auto&& uboManager = Memory::UBOManager::Instance();
        if (m_virtualUBO != SR_ID_INVALID && !uboManager.FreeUBO(&m_virtualUBO)) {
            SR_ERROR("Mesh::FreeVideoMemory() : failed to free virtual uniform buffer object!");
        }

        SetShader(nullptr);

        IGraphicsResource::FreeVideoMemory();
    }

    void Skybox::Draw() {
        if (!m_isCalculated && (m_hasErrors || !Calculate())) {
            return;
        }

        switch (m_pipeline->GetType()) {
            case PipelineType::Vulkan:
                DrawVulkan();
                break;
            default:
                SRAssertOnce(false);
                break;
        }
    }

    void Skybox::OnResourceUpdated(SR_UTILS_NS::ResourceContainer* pContainer, int32_t depth) {
        if (dynamic_cast<Shader*>(pContainer) == m_shader && m_shader) {
            m_dirtyShader = true;
            m_hasErrors = false;
        }

        IResource::OnResourceUpdated(pContainer, depth);
    }

    void Skybox::SetShader(Shader *shader) {
        SR_SCOPED_LOCK

        if (m_shader == shader) {
            return;
        }

        m_dirtyShader = true;

        if (m_shader) {
            RemoveDependency(m_shader);
            m_shader = nullptr;
        }

        if (!(m_shader = shader)) {
            return;
        }

        AddDependency(m_shader);
    }

    int32_t Skybox::GetVBO() {
        if (!m_isCalculated && (m_hasErrors || !Calculate())) {
            return SR_ID_INVALID;
        }

        return m_VBO;
    }

    int32_t Skybox::GetIBO() {
        if (!m_isCalculated && (m_hasErrors || !Calculate())) {
            return SR_ID_INVALID;
        }

        return m_IBO;
    }

    int32_t Skybox::GetVirtualUBO() {
        return m_virtualUBO;
    }
}