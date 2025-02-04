//
// Created by Igor on 27/11/2022.
//

#include <Graphics/Types/Geometry/SkinnedMesh.h>

namespace SR_GTYPES_NS {
    SR_REGISTER_COMPONENT(SkinnedMesh);

    SkinnedMesh::SkinnedMesh()
        : Super(MeshType::Skinned)
    { }

    SkinnedMesh::~SkinnedMesh() {
        SetRawMesh(nullptr);
    }

    bool SkinnedMesh::Calculate()  {
        SR_LOCK_GUARD_INHERIT(SR_UTILS_NS::IResource);

        if (m_isCalculated)
            return true;

        const bool iboOK = m_IBO != SR_ID_INVALID;
        if (m_VBO != SR_ID_INVALID && iboOK && !m_hasErrors) {
            m_isCalculated = true;
            return true;
        }

        if (!IsCanCalculate()) {
            return false;
        }

        if (SR_UTILS_NS::Debug::Instance().GetLevel() >= SR_UTILS_NS::Debug::Level::High) {
            SR_LOG("SkinnedMesh::Calculate() : calculating \"" + m_geometryName + "\"...");
        }

        auto&& vertices = Vertices::CastVertices<Vertices::SkinnedMeshVertex>(m_rawMesh->GetVertices(m_meshId));

        if (!CalculateVBO<Vertices::VertexType::SkinnedMeshVertex>(vertices))
            return false;

        return IndexedMesh::Calculate();
    }

    SR_UTILS_NS::IResource* SkinnedMesh::CopyResource(IResource* destination) const {
        SR_LOCK_GUARD_INHERIT(SR_UTILS_NS::IResource);

        auto* pSkinnedMesh = dynamic_cast<SkinnedMesh *>(destination ? destination : new SkinnedMesh());
        pSkinnedMesh = dynamic_cast<SkinnedMesh *>(IndexedMesh::CopyResource(pSkinnedMesh));

        pSkinnedMesh->SetRawMesh(m_rawMesh);
        pSkinnedMesh->m_meshId = m_meshId;

        return pSkinnedMesh;
    }

    void SkinnedMesh::FreeVideoMemory() {
        SR_LOCK_GUARD_INHERIT(SR_UTILS_NS::IResource);

        if (SR_UTILS_NS::Debug::Instance().GetLevel() >= SR_UTILS_NS::Debug::Level::High) {
            SR_LOG("SkinnedMesh::FreeVideoMemory() : free \"" + m_geometryName + "\" mesh video memory...");
        }

        if (!FreeVBO<Vertices::VertexType::SkinnedMeshVertex>()) {
            SR_ERROR("SkinnedMesh::FreeVideoMemory() : failed to free VBO!");
        }

        IndexedMesh::FreeVideoMemory();
    }

    void SkinnedMesh::Draw() {
        auto&& pShader = GetRenderContext()->GetCurrentShader();

        if (!pShader || !IsActive() || IsDestroyed() || !IsSkeletonUsable())
            return;

        if ((!m_isCalculated && !Calculate()) || m_hasErrors)
            return;

        if (m_dirtyMaterial)
        {
            m_dirtyMaterial = false;

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

            m_material->UseSamplers();
        }

        switch (m_uboManager.BindUBO(m_virtualUBO)) {
            case Memory::UBOManager::BindResult::Duplicated:
                pShader->InitUBOBlock();
                pShader->Flush();
                m_material->UseSamplers();
                SR_FALLTHROUGH;
            case Memory::UBOManager::BindResult::Success:
                m_pipeline->DrawIndices(m_countIndices);
                break;
            case Memory::UBOManager::BindResult::Failed:
            default:
                break;
        }
    }

    SR_HTYPES_NS::Marshal::Ptr SR_GTYPES_NS::SkinnedMesh::Save(SR_HTYPES_NS::Marshal::Ptr pMarshal, SR_UTILS_NS::SavableFlags flags) const {
        pMarshal = Component::Save(pMarshal, flags);

        pMarshal->Write(static_cast<int32_t>(m_type));

        /// TODO: use unicode
        pMarshal->Write(GetResourcePath().ToString());
        pMarshal->Write(m_meshId);

        pMarshal->Write(m_material ? m_material->GetResourceId() : "None");

        return pMarshal;
    }

    SR_UTILS_NS::Component *SR_GTYPES_NS::SkinnedMesh::LoadComponent(SR_HTYPES_NS::Marshal& marshal, const SR_HTYPES_NS::DataStorage *dataStorage) {
        const auto &&type = static_cast<MeshType>(marshal.Read<int32_t>());

        const auto &&path = marshal.Read<std::string>();
        const auto &&id = marshal.Read<uint32_t>();

        const auto &&material = marshal.Read<std::string>();

        if (id < 0) {
            return nullptr;
        }

        auto &&pMesh = Mesh::Load(SR_UTILS_NS::Path(path, true), type, id);

        if (pMesh && material != "None") {
            if (auto&& pMaterial = Types::Material::Load(SR_UTILS_NS::Path(material, true))) {
                pMesh->SetMaterial(pMaterial);
            }
            else
                SR_ERROR("SkinnedMesh::LoadComponent() : failed to load material! Name: " + material);
        }

        return dynamic_cast<Component*>(pMesh);
    }

    bool SkinnedMesh::Unload() {
        SetRawMesh(nullptr);
        return true;
    }

    bool SkinnedMesh::Load() {
        if (auto&& pRawMesh = SR_HTYPES_NS::RawMesh::Load(GetResourcePath())) {
            if (m_meshId >= pRawMesh->GetMeshesCount()) {
                if (pRawMesh->GetCountUses() == 0) {
                    SRHalt("SkinnedMesh::Load() : count uses is zero! Unresolved situation...");
                    pRawMesh->Destroy();
                }
                return false;
            }

            SetRawMesh(pRawMesh);

            m_countIndices = pRawMesh->GetIndicesCount(m_meshId);
            m_countVertices = pRawMesh->GetVerticesCount(m_meshId);

            SetGeometryName(pRawMesh->GetGeometryName(m_meshId));

            return true;
        }

        return false;
    }

    void SkinnedMesh::SetRawMesh(SR_HTYPES_NS::RawMesh *pRaw) {
        if (m_rawMesh) {
            RemoveDependency(m_rawMesh);
        }

        if (pRaw) {
            AddDependency(pRaw);
        }

        m_rawMesh = pRaw;
    }

    std::vector<uint32_t> SkinnedMesh::GetIndices() const {
        return m_rawMesh->GetIndices(m_meshId);
    }

    bool SkinnedMesh::IsCanCalculate() const {
        return m_rawMesh && m_meshId < m_rawMesh->GetMeshesCount() && Mesh::IsCanCalculate();
    }

    bool SkinnedMesh::IsSkeletonUsable() const {
        if (m_skeleton)
            return m_skeleton->GetRootBone();
        else
            return false;
    }

    void SkinnedMesh::Update(float dt) {
        FindSkeleton(GetGameObject());
        Super::Update(dt);
    };

    void SkinnedMesh::UseMaterial() {
        Super::UseMaterial();
        UseModelMatrix();
    }

    void SkinnedMesh::UseModelMatrix() {
        PopulateSkeletonMatrices(); ///TODO:А не стоило бы изменить ColorBufferPass так, чтобы он вызывал не UseModelMatrix, а более обощённый метод?
        GetRenderContext()->GetCurrentShader()->SetCustom(SHADER_SKELETON_MATRICES_128, m_skeletonMatrices.data());
        GetRenderContext()->GetCurrentShader()->SetCustom(SHADER_SKELETON_MATRIX_OFFSETS_128, m_skeletonOffsets.data());
        GetRenderContext()->GetCurrentShader()->SetMat4(SHADER_MODEL_MATRIX, m_modelMatrix);
    }

    void SkinnedMesh::PopulateSkeletonMatrices() {
        static SR_MATH_NS::Matrix4x4 identityMatrix = SR_MATH_NS::Matrix4x4().Identity();

        auto&& bones = m_rawMesh->GetBones(m_meshId);

        if (m_bonesIds.empty()) {
            const uint64_t bonesCount = SR_MAX(SR_HUMANOID_MAX_BONES, bones.size());

            m_bonesIds.resize(bonesCount);
            m_skeletonOffsets.resize(bonesCount);
            m_skeletonMatrices.resize(bonesCount);

            for (uint64_t i = 0; i < m_bonesIds.size(); i++) {
                m_skeletonMatrices[i] = identityMatrix;
            }
        }

        if (!m_skeleton) {
            if (!m_isSkeletonDeleted) {
                m_isSkeletonDeleted = true;
                m_renderScene->SetDirty();
            }
            return;
        } else {
            if (!m_skeleton->GetRootBone())
                return;
            if (m_isSkeletonDeleted) {
                m_isSkeletonDeleted = false;
                m_renderScene->SetDirty();
            }
        }

        /*if (!IsSkeletonUsable()) {
            for (uint64_t i = 0; i < m_bonesIds.size(); i++) {
                m_skeletonMatrices[i] = identityMatrix;
                m_skeletonOffsets[i] = identityMatrix;
            }
            m_isOffsetsInitialized = false;

            return;
        }*/

        if (!m_isOffsetsInitialized) {
            for (auto&& [hashName, boneId] : bones) {
                m_skeletonOffsets[boneId] = m_rawMesh->GetBoneOffset(hashName);
                m_bonesIds[boneId] = m_skeleton->GetBoneIndex(hashName);
            }
            m_isOffsetsInitialized = true;
        }

        if (m_skeleton)
        for (uint64_t boneId = 0; boneId < m_bonesIds.size(); ++boneId) {
            if (auto&& bone = m_skeleton->GetBoneByIndex(m_bonesIds[boneId])) {
                m_skeletonMatrices[boneId] = bone->gameObject->GetTransform()->GetMatrix();
            }
            else {
                m_skeletonMatrices[boneId] = identityMatrix;
            }
        }

    }

    void SkinnedMesh::FindSkeleton(SR_UTILS_NS::GameObject::Ptr gameObject) {
        m_skeleton = dynamic_cast<Animations::Skeleton *>(gameObject->GetComponent("Skeleton"));
        if (!m_skeleton && gameObject->GetParent()) {
            FindSkeleton(gameObject->GetParent());
        }
    }
}