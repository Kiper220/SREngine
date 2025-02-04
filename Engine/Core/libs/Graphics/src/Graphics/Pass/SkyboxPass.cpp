//
// Created by Monika on 20.07.2022.
//

#include <Graphics/Pass/SkyboxPass.h>
#include <Graphics/Types/Skybox.h>
#include <Graphics/Types/Shader.h>
#include <Graphics/Pipeline/IShaderProgram.h>

namespace SR_GRAPH_NS {
    SR_REGISTER_RENDER_PASS(SkyboxPass)

    SkyboxPass::SkyboxPass(RenderTechnique *pTechnique, BasePass* pParent)
        : BasePass(pTechnique, pParent)
    { }

    SkyboxPass::~SkyboxPass() {
        if (m_skybox) {
            m_skybox->RemoveUsePoint();
        }
    }

    bool SkyboxPass::Load(const SR_XML_NS::Node &passNode) {
        auto&& path = passNode.GetAttribute<SR_UTILS_NS::Path>();

        if (m_skybox) {
            m_skybox->RemoveUsePoint();
            m_skybox = nullptr;
        }

        if (!(m_skybox = SR_GTYPES_NS::Skybox::Load(path))) {
            SR_ERROR("SkyboxPass::Load() : failed to load skybox!\n\tPath: " + path.ToString());
            return false;
        }
        else {
            m_skybox->AddUsePoint();
        }

        if (m_skybox) {
            m_skybox->SetShader(SR_GTYPES_NS::Shader::Load(passNode.GetAttribute("Shader").ToString()));
        }

        return BasePass::Load(passNode);
    }

    bool SkyboxPass::Render() {
        if (!m_skybox) {
            return false;
        }

        auto&& shader = m_skybox->GetShader();

        if (!shader || !shader->Use()) {
            return false;
        }

        m_skybox->Draw();

        shader->UnUse();

        return true;
    }

    void SkyboxPass::Update() {
        if (!m_skybox) {
            return;
        }

        auto&& pShader = m_skybox->GetShader();

        if (!pShader || !pShader->Ready() || !m_camera) {
            return;
        }

        pShader->SetMat4(SHADER_VIEW_NO_TRANSLATE_MATRIX, m_camera->GetViewRef());
        pShader->SetMat4(SHADER_PROJECTION_MATRIX, m_camera->GetProjectionRef());

        auto&& virtualUbo = m_skybox->GetVirtualUBO();
        if (virtualUbo == SR_ID_INVALID) {
            return;
        }

        m_context->SetCurrentShader(pShader);

        if (m_uboManager.BindUBO(virtualUbo) == Memory::UBOManager::BindResult::Duplicated) {
            SR_ERROR("SkyboxPass::Update() : memory has been duplicated!");
        }

        pShader->Flush();

        BasePass::Update();
    }

    bool SkyboxPass::Init() {
        bool result = BasePass::Init();

        if (m_skybox) {
            m_context->Register(m_skybox);
        }

        return result;
    }
}