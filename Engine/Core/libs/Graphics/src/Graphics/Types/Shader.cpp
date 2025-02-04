//
// Created by Nikita on 17.11.2020.
//

#include <Utils/ResourceManager/ResourceManager.h>
#include <Utils/Xml.h>
#include <Utils/Types/Thread.h>
#include <Utils/Types/DataStorage.h>
#include <Utils/Common/Hashes.h>

#include <Graphics/Types/Texture.h>
#include <Graphics/Render/Render.h>
#include <Graphics/Render/RenderContext.h>
#include <Graphics/Pipeline/Environment.h>
#include <Graphics/Types/Shader.h>
#include <Graphics/SRSL/Shader.h>
#include <Graphics/SRSL/TypeInfo.h>

namespace SR_GRAPH_NS::Types {
    Shader::Shader()
        : IResource(SR_COMPILE_TIME_CRC32_TYPE_NAME(Shader), true /** auto remove */)
    { }

    Shader::~Shader() {
        m_uniformBlock.DeInit();
        m_uniformBlock = Memory::ShaderUBOBlock();

        m_samplers.clear();
    }

    bool Shader::Init() {
        if (m_isInit) {
            SRHalt("Double shader initialization!");
            return true;
        }

        auto&& context = SR_THIS_THREAD->GetContext()->GetValue<SR_HTYPES_NS::SafePtr<RenderContext>>();
        if (!context) {
            SRHalt("Is not render context!");
            m_hasErrors = true;
            return false;
        }

        if (!m_isRegistered && context.LockIfValid()) {
            context->Register(this);
            context.Unlock();
            m_isRegistered = true;
        }

        if (!m_shaderCreateInfo.Validate()) {
            SR_ERROR("Shader::Init() : failed to validate shader!\n\tPath: " + GetResourcePath().ToString());
            m_hasErrors = true;
            return false;
        }

        m_shaderProgram = Memory::ShaderProgramManager::Instance().ReAllocate(m_shaderProgram, m_shaderCreateInfo);
        if (m_shaderProgram == SR_ID_INVALID) {
            SR_ERROR("Shader::Init() : failed to allocate shader program!");
            m_hasErrors = true;
            return false;
        }

        /// calculate shader params hash
        {
            auto&& hash = SR_UTILS_NS::HashCombine(m_properties, 0);
            hash = SR_UTILS_NS::HashCombine(m_samplers, hash);
            SetResourceHash(hash);
        }

        m_isInit = true;

        return true;
    }

    bool Shader::Use() noexcept {
        if (m_hasErrors) {
            return false;
        }

        if (!m_isInit && !Init()) {
            SR_ERROR("Shader::Use() : failed to initialize shader!");
            return false;
        }

        switch (Memory::ShaderProgramManager::Instance().BindProgram(m_shaderProgram)) {
            case Memory::ShaderProgramManager::BindResult::Success:
            case Memory::ShaderProgramManager::BindResult::Duplicated:
                GetRenderContext()->SetCurrentShader(this);
                return true;
            case Memory::ShaderProgramManager::BindResult::Failed:
            default:
                return false;
        }
    }

    void Shader::UnUse() noexcept {
        auto&& env = SR_GRAPH_NS::Environment::Get();
        env->UnUseShader();

        if (GetRenderContext()->GetCurrentShader() == this) {
            GetRenderContext()->SetCurrentShader(nullptr);
        }
    }

    void Shader::FreeVideoMemory() {
        if (m_isInit) {
            SR_SHADER("Shader::FreeVideoMemory() : free \"" + GetResourceId() + "\" video memory...");

            if (!Memory::ShaderProgramManager::Instance().FreeProgram(&m_shaderProgram)) {
                SR_ERROR("Shader::Free() : failed to free shader program! \n\tPath: " + GetResourcePath().ToString());
            }
        }
    }

    Shader* Shader::Load(const SR_UTILS_NS::Path &rawPath) {
        auto&& resourceManager = SR_UTILS_NS::ResourceManager::Instance();

        SR_UTILS_NS::Path&& path = SR_UTILS_NS::Path(rawPath).RemoveSubPath(resourceManager.GetResPath());

        if (auto&& pShader = resourceManager.Find<Shader>(path)) {
            return pShader;
        }

        SR_LOG("Shader::Load() : load \"" + path.ToString() + "\" shader...");

        if (!SRVerifyFalse2(path.Empty(), "Invalid shader path!")) {
            SR_WARN("Shader::Load() : failed to load shader!");
            return nullptr;
        }

        if (path.GetExtensionView() != "srsl") {
            SR_ERROR("Shader::Load() : unknown extension!");
            return nullptr;
        }

        auto&& pShader = new Shader();

        pShader->SetId(path.ToString(), false);

        if (!pShader->Reload()) {
            SR_ERROR("Shader::Load() : failed to reload shader!\n\tPath: " + path.ToString());
            delete pShader;
            return nullptr;
        }

        resourceManager.RegisterResource(pShader);

        return pShader;
    }

    int32_t Shader::GetID() {
        return GetId();
    }

    int32_t Shader::GetId() noexcept {
        if (m_hasErrors) {
            return false;
        }

        if (!m_isInit && !Init()) {
            SR_ERROR("Shader::Use() : failed to initialize shader!");
            return false;
        }

        return Memory::ShaderProgramManager::Instance().GetProgram(m_shaderProgram);
    }

    void Shader::SetBool(uint64_t hashId, const bool &v) noexcept { SetValue(hashId, v); }
    void Shader::SetFloat(uint64_t hashId, const float &v) noexcept { SetValue(hashId, v); }
    void Shader::SetInt(uint64_t hashId, const int &v) noexcept { SetValue(hashId, v); }
    void Shader::SetMat4(uint64_t hashId, const glm::mat4 &v) noexcept { SetValue(hashId, v); }
    void Shader::SetMat4(uint64_t hashId, const SR_MATH_NS::Matrix4x4& v) noexcept { SetValue(hashId, v); }
    void Shader::SetVec3(uint64_t hashId, const SR_MATH_NS::FVector3& v) noexcept { SetValue(hashId, v); }
    void Shader::SetVec4(uint64_t hashId, const SR_MATH_NS::FVector4& v) noexcept { SetValue(hashId, v); }
    void Shader::SetVec2(uint64_t hashId, const SR_MATH_NS::FVector2& v) noexcept { SetValue(hashId, v); }
    void Shader::SetIVec2(uint64_t hashId, const glm::ivec2 &v) noexcept { SetValue(hashId, v); }

    void Shader::SetSampler(uint64_t hashId, int32_t sampler) noexcept {
        auto&& env = SR_GRAPH_NS::Environment::Get();
        env->BindTexture(m_samplers.at(hashId), sampler);
    }

    void Shader::SetSampler2D(uint64_t hashId, int32_t sampler) noexcept {
        if (!IsLoaded() || m_samplers.count(hashId) == 0) {
            return;
        }

        SetSampler(hashId, sampler);
    }

    void Shader::SetSamplerCube(uint64_t hashId, int32_t sampler) noexcept {
        if (!IsLoaded() || m_samplers.count(hashId) == 0) {
            return;
        }

        SetSampler(hashId, sampler);
    }

    void Shader::SetSampler2D(uint64_t hashId, Types::Texture *sampler) noexcept {
        if (!IsLoaded() || m_samplers.count(hashId) == 0) {
            return;
        }

        if (!sampler) {
            sampler = GetRenderContext()->GetNoneTexture();
        }

        if (!sampler) {
            SRHalt("The sampler is nullptr!");
            return;
        }

        SetSampler(hashId, sampler->GetId());
    }

    void Shader::SetSampler2D(const std::string &name, Types::Texture *sampler) noexcept {
        SetSampler2D(SR_RUNTIME_TIME_CRC32_STR(name.c_str()), sampler);
    }

    bool Shader::Ready() const {
        return !m_hasErrors && m_isInit && m_shaderProgram != SR_ID_INVALID;
    }

    uint64_t Shader::GetUBOBlockSize() const {
        return m_uniformBlock.m_size;
    }

    bool Shader::InitUBOBlock() {
        auto&& env = SR_GRAPH_NS::Environment::Get();

        if (m_uniformBlock.m_size > 0 && m_uniformBlock.m_memory) {
            memset(m_uniformBlock.m_memory, 1, m_uniformBlock.m_size);
        }

        auto&& ubo = env->GetCurrentUBO();
        auto&& descriptorSet = env->GetCurrentDescriptorSet();

        if (ubo != SR_ID_INVALID && descriptorSet != SR_ID_INVALID && m_uniformBlock.Valid()) {
            env->UpdateDescriptorSets(descriptorSet, {
                    { DescriptorType::Uniform, { m_uniformBlock.m_binding, ubo } },
            });

            return true;
        }

        return false;
    }

    bool Shader::Flush() const {
        if (!m_isInit || m_hasErrors) {
            return false;
        }

        auto&& env = SR_GRAPH_NS::Environment::Get();

        auto &&ubo = env->GetCurrentUBO();
        if (ubo != SR_ID_INVALID && m_uniformBlock.Valid()) {
            env->UpdateUBO(ubo, m_uniformBlock.m_memory, m_uniformBlock.m_size);
        }

        return true;
    }

    uint32_t Shader::GetSamplersCount() const {
        return m_samplers.size();
    }

    ShaderProperties Shader::GetProperties() {
        return m_properties;
    }

    SR_UTILS_NS::Path Shader::GetAssociatedPath() const {
        return SR_UTILS_NS::ResourceManager::Instance().GetResPath();
    }

    void Shader::OnReloadDone() {
        auto&& pContext = GetRenderContext();
        if (!pContext) {
            return;
        }

        /// пока ресурс жив, контекст будет существовать (если ресурс зарегистрирован)
        pContext->Do([](RenderContext* ptr) {
            ptr->SetDirty();
        });
    }

    bool Shader::Load() {
        SR_LOCK_GUARD

        SR_UTILS_NS::Path&& path = SR_UTILS_NS::Path(GetResourceId());

        if (path.IsAbs()) {
            SR_ERROR("Shader::Load() : absolute path is not allowed!");
            return false;
        }

        auto&& pShader = SR_SRSL_NS::SRSLShader::Load(path);
        if (!pShader) {
            SR_ERROR("Shader::Load() : failed to load srsl shader!\n\tPath: " + path.ToString());
            return false;
        }

        if (!pShader->Export(SRSL2::ShaderLanguage::GLSL)) {
            SR_ERROR("Shader::Load() : failed to export srsl shader!\n\tPath: " + path.ToString());
            return false;
        }

        m_shaderCreateInfo = SRShaderCreateInfo(pShader->GetCreateInfo());
        m_type = pShader->GetType();

        if (auto&& pBlock = pShader->FindUniformBlock("BLOCK")) {
            for (auto&& field : pBlock->fields) {
                m_uniformBlock.Append(SR_RUNTIME_TIME_CRC32_STR(field.name.c_str()), field.size, field.alignedSize, !field.isPublic);

                const ShaderVarType varType = SR_SRSL_NS::SRSLTypeInfo::Instance().StringToType(field.type);

                if (field.isPublic && varType != ShaderVarType::Unknown) {
                    m_properties.emplace_back(std::make_pair(field.name, varType));
                }
            }

            m_uniformBlock.m_binding = pBlock->binding;
        }

        for (auto&& [name, sampler] : pShader->GetSamplers()) {
            m_samplers[SR_RUNTIME_TIME_CRC32_STR(name.c_str())] = sampler.binding;

            const ShaderVarType varType = SR_SRSL_NS::SRSLTypeInfo::Instance().StringToType(sampler.type);

            if (sampler.isPublic && varType != ShaderVarType::Unknown) {
                m_properties.emplace_back(std::make_pair(name, varType));
            }
        }

        m_uniformBlock.Init();

        return IResource::Load();
    }

    bool Shader::Unload() {
        SR_LOCK_GUARD

        bool hasErrors = !IResource::Unload();

        m_isInit = false;
        m_hasErrors = false;

        m_uniformBlock.DeInit();

        m_properties.clear();
        m_samplers.clear();

        return !hasErrors;
    }

    bool Shader::IsBlendEnabled() const {
        return m_shaderCreateInfo.blendEnabled;
    }

    SR_SRSL_NS::ShaderType Shader::GetType() const noexcept {
        return m_type;
    }
}