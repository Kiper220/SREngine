//
// Created by Monika on 19.09.2022.
//

#include <Graphics/Pass/DebugPass.h>
#include <Graphics/Types/Material.h>
#include <Graphics/Types/Shader.h>
#include <Graphics/Types/Geometry/IndexedMesh.h>
#include <Graphics/Render/MeshCluster.h>
#include <Graphics/Pipeline/IShaderProgram.h>

namespace SR_GRAPH_NS {
    SR_REGISTER_RENDER_PASS(DebugPass)

    DebugPass::DebugPass(RenderTechnique *pTechnique, BasePass* pParent)
        : BasePass(pTechnique, pParent)
    { }

    bool DebugPass::PreRender() {
        return false;
    }

    bool DebugPass::Render() {
        auto&& debug = GetRenderScene()->GetDebugCluster();

        if (debug.Empty()) {
            return false;
        }

        for (auto&& [shader, subCluster] : debug) {
            if (!shader || (shader && !shader->Use())) {
                continue;
            }

            for (auto&& [key, meshGroup] : subCluster) {
                (*meshGroup.begin())->BindMesh();

                for (auto&& pMesh : meshGroup) {
                    pMesh->Draw();
                }
            }

            shader->UnUse();
        }

        return true;
    }

    void DebugPass::Update() {
        if (!m_camera) {
            return;
        }

        auto&& debug = GetRenderScene()->GetDebugCluster();
        auto&& time = clock();

        for (auto const& [pShader, subCluster] : debug) {
            if (!pShader || !pShader->Ready()) {
                continue;
            }

            m_context->SetCurrentShader(pShader);

            /**
             * TODO: нужно сделать что-то вроде SetSharedMat4, который будет биндить не в BLOCK а в SHARED_BLOCK
             */
            pShader->SetMat4(SHADER_VIEW_MATRIX, m_camera->GetViewTranslateRef());
            pShader->SetMat4(SHADER_PROJECTION_MATRIX, m_camera->GetProjectionRef());
            pShader->SetMat4(SHADER_ORTHOGONAL_MATRIX, m_camera->GetOrthogonalRef());
            pShader->SetFloat(SHADER_TIME, time);

            for (auto const& [key, meshGroup] : subCluster) {
                for (const auto &mesh : meshGroup) {
                    if (!mesh->IsMeshActive()) {
                        continue;
                    }

                    auto&& virtualUbo = mesh->GetVirtualUBO();
                    if (virtualUbo == SR_ID_INVALID) {
                        continue;
                    }

                    mesh->UseMaterial();

                    pShader->SetVec3(SHADER_VIEW_DIRECTION, m_camera->GetViewDirection());

                    if (m_uboManager.BindUBO(virtualUbo) == Memory::UBOManager::BindResult::Duplicated) {
                        SR_ERROR("DebugPass::Update() : memory has been duplicated!");
                    }

                    pShader->Flush();
                }
            }
        }

        BasePass::Update();
    }
}