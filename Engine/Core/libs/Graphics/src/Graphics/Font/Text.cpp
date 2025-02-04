//
// Created by Monika on 14.02.2022.
//

#include <Utils/ECS/Transform.h>
#include <Utils/ECS/ComponentManager.h>

#include <Graphics/Font/Text.h>
#include <Graphics/Font/TextBuilder.h>

namespace SR_GTYPES_NS {
    SR_REGISTER_COMPONENT(Text);

    Text::Text()
        : Super(MeshType::Static)
    {
        SetMaterial(Material::Load("Engine/Materials/text.mat"));
        SetFont(Font::Load("Engine/Fonts/TsunagiGothic.ttf"));
        //SetFont(Font::Load("Engine/Fonts/seguiemj.ttf"));
        //m_text = U"Heприветlloあにま😀 😬 😁 😂 😃 😄 😅 😆 😇 😉 😊 🙂 🙃 ☺️ \n😋 😌 👦🏻 👧🏻 👨🏻 👩🏻 👴🏻 👵🏻 👶🏻 👱🏻 👮🏻 👲🏻 👳🏻 👷🏻 👸🏻 💂🏻 🎅🏻 👼🏻 💆🏻 💇🏻\n🤣 🤠 🤡 🤥 🤤 🤢";
        m_text = U"Hello, World!";
        //m_text = U"Hello!";
    }

    Text::~Text() {
        SetFont(nullptr);
    }

    void Text::Draw() {
        auto&& pShader = GetRenderContext()->GetCurrentShader();

        if (!pShader || !IsActive() || IsDestroyed()) {
            return;
        }

        if ((!m_isCalculated && !Calculate()) || m_hasErrors) {
            return;
        }

        if (m_dirtyMaterial)
        {
            m_dirtyMaterial = false;

            EVK_PUSH_LOG_LEVEL(EvoVulkan::Tools::LogLevel::ErrorsOnly);

            m_virtualUBO = m_uboManager.ReAllocateUBO(m_virtualUBO, pShader->GetUBOBlockSize(), pShader->GetSamplersCount());

            if (m_virtualUBO != SR_ID_INVALID) {
                m_uboManager.BindUBO(m_virtualUBO);
            }
            else {
                m_pipeline->ResetDescriptorSet();
                m_hasErrors = true;
                return;
            }

            pShader->InitUBOBlock();
            pShader->Flush();

            UseSamplers();

            EVK_POP_LOG_LEVEL();
        }

        switch (m_uboManager.BindUBO(m_virtualUBO)) {
            case Memory::UBOManager::BindResult::Duplicated:
                pShader->InitUBOBlock();
                pShader->Flush();
                UseSamplers();
                SR_FALLTHROUGH;
            case Memory::UBOManager::BindResult::Success:
                m_pipeline->DrawIndices(6);
                break;
            case Memory::UBOManager::BindResult::Failed:
            default:
                break;
        }
    }

    SR_UTILS_NS::IResource* Text::CopyResource(SR_UTILS_NS::IResource* destination) const{
        return Mesh::CopyResource(destination);
    }

    bool Text::Calculate() {
        SR_LOCK_GUARD_INHERIT(SR_UTILS_NS::IResource);

        if (m_isCalculated) {
            return true;
        }

        if (m_hasErrors || !IsCanCalculate()) {
            return false;
        }

        if (!BuildAtlas()) {
            SR_ERROR("Text::Calculate() : failed to build atlas!");
            return false;
        }

        return Mesh::Calculate();
    }

    void Text::FreeVideoMemory() {
        SR_LOCK_GUARD_INHERIT(SR_UTILS_NS::IResource);

        if (m_id != SR_ID_INVALID) {
            SRVerifyFalse(!m_pipeline->FreeTexture(&m_id));
        }

        Mesh::FreeVideoMemory();
    }

    bool Text::BuildAtlas() {
        if (!m_font) {
            SR_ERROR("Text::BuildAtlas() : missing font!");
            return false;
        }

        EVK_PUSH_LOG_LEVEL(EvoVulkan::Tools::LogLevel::ErrorsOnly);

        if (m_id != SR_ID_INVALID) {
            SRVerifyFalse(!m_pipeline->FreeTexture(&m_id));
        }

        TextBuilder textBuilder(m_font);
        textBuilder.Build(m_text);

        m_width = textBuilder.GetWidth();
        m_height = textBuilder.GetHeight();

        m_id = m_pipeline->CalculateTexture(textBuilder.GetData(),
                ColorFormat::RGBA8_SRGB, m_width, m_height, TextureFilter::NEAREST,
                TextureCompression::None, 1,
                true, false
        );

        EVK_POP_LOG_LEVEL();

        if (m_id == SR_ID_INVALID) {
            SR_ERROR("Text::BuildAtlas() : failed to build the font atlas!");
            return false;
        }

        return true;
    }

    SR_UTILS_NS::Component* Text::LoadComponent(SR_HTYPES_NS::Marshal& marshal, const SR_HTYPES_NS::DataStorage* dataStorage) {
        SR_MAYBE_UNUSED const auto &&type = static_cast<MeshType>(marshal.Read<int32_t>());

        const auto &&material = marshal.Read<std::string>();
        auto &&pMesh = new Text();

        if (pMesh && material != "None") {
            if (auto&& pMaterial = Types::Material::Load(SR_UTILS_NS::Path(material, true))) {
                pMesh->SetMaterial(pMaterial);
            }
            else {
                SR_ERROR("Text::LoadComponent() : failed to load material! Name: " + material);
            }
        }

        return dynamic_cast<Component*>(pMesh);
    }

    void Text::UseMaterial() {
        Mesh::UseMaterial();
        UseModelMatrix();
    }

    void Text::UseModelMatrix() {
        GetRenderContext()->GetCurrentShader()->SetMat4(SHADER_MODEL_MATRIX, m_modelMatrix);
        GetRenderContext()->GetCurrentShader()->SetFloat(SHADER_TEXT_RECT_X, 0.f);
        GetRenderContext()->GetCurrentShader()->SetFloat(SHADER_TEXT_RECT_Y, 0.f);
        GetRenderContext()->GetCurrentShader()->SetFloat(SHADER_TEXT_RECT_WIDTH, m_width / 100.f);
        GetRenderContext()->GetCurrentShader()->SetFloat(SHADER_TEXT_RECT_HEIGHT, m_height / 100.f);

        Mesh::UseModelMatrix();
    }

    void Types::Text::OnMatrixDirty() {
        if (auto&& pTransform = GetTransform()) {
            m_modelMatrix = pTransform->GetMatrix();
        }
        else {
            m_modelMatrix = SR_MATH_NS::Matrix4x4::Identity();
        }

        Component::OnMatrixDirty();
    }

    Mesh::RenderScenePtr Text::GetRenderScene() {
        if (!m_renderScene.Valid()) {
            m_renderScene = TryGetScene()->Do<RenderScenePtr>([](SR_WORLD_NS::Scene* ptr) {
                return ptr->GetDataStorage().GetValue<RenderScenePtr>();
            }, RenderScenePtr());
        }

        return m_renderScene;
    }

    void Text::OnEnable() {
        if (auto&& renderScene = GetRenderScene()) {
            renderScene->SetDirty();
        }
        Component::OnEnable();
    }

    void Text::OnDisable() {
        if (auto&& renderScene = GetRenderScene()) {
            renderScene->SetDirty();
        }
        Component::OnDisable();
    }

    void Text::OnAttached() {
        GetRenderScene().Do([this](SR_GRAPH_NS::RenderScene *ptr) {
            ptr->Register(this);
        });

        Component::OnAttached();
    }

    void Text::OnDestroy() {
        Component::OnDestroy();

        auto&& renderScene = GetRenderScene();

        /// после вызова данная сущность может быть уничтожена
        RemoveUsePoint();

        renderScene->SetDirty();
    }

    SR_NODISCARD SR_HTYPES_NS::Marshal::Ptr Text::Save(SR_HTYPES_NS::Marshal::Ptr pMarshal, SR_UTILS_NS::SavableFlags flags) const {
        pMarshal = Component::Save(pMarshal, flags);

        pMarshal->Write(static_cast<int32_t>(m_type));
        pMarshal->Write(m_material ? m_material->GetResourceId() : "None");

        return pMarshal;
    }

    void Text::UseSamplers() {
        GetRenderContext()->GetCurrentShader()->SetSampler2D(SHADER_TEXT_ATLAS_TEXTURE, m_id);
        Mesh::UseSamplers();
    }

    void Text::SetFont(Font *pFont) {
        if (pFont == m_font) {
            return;
        }

        if (m_font) {
            RemoveDependency(m_font);
        }

        if ((m_font = pFont)) {
            AddDependency(m_font);
        }
    }

    void Text::SetText(const std::string &text) {
        m_text = SR_UTILS_NS::Locale::UtfToUtf<char32_t, char>(text);
        m_isCalculated = false;
        m_dirtyMaterial = true;
        if (auto&& renderScene = GetRenderScene()) {
            renderScene->SetDirty();
        }
    }

    void Text::SetText(const std::u16string &text) {
        m_text = SR_UTILS_NS::Locale::UtfToUtf<char32_t, char16_t>(text);
        m_isCalculated = false;
        m_dirtyMaterial = true;
        if (auto&& renderScene = GetRenderScene()) {
            renderScene->SetDirty();
        }
    }

    void Text::SetText(const std::u32string &text) {
        m_text = text;
        m_isCalculated = false;
        m_dirtyMaterial = true;
        if (auto&& renderScene = GetRenderScene()) {
            renderScene->SetDirty();
        }
    }

    bool Text::IsCanCalculate() const {
        return Mesh::IsCanCalculate() && !m_text.empty();
    }

    void Text::OnLoaded() {
        AddUsePoint();
        Component::OnLoaded();
    }
}