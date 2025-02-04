//
// Created by Nikita on 17.11.2020.
//

#ifndef GAMEENGINE_MESH_H
#define GAMEENGINE_MESH_H

#include <Utils/Math/Matrix4x4.h>
#include <Utils/Common/Enumerations.h>
#include <Utils/ResourceManager/IResource.h>

#include <Graphics/Pipeline/PipelineType.h>
#include <Graphics/Memory/IGraphicsResource.h>
#include <Graphics/Memory/UBOManager.h>

namespace SR_HTYPES_NS {
    class RawMesh;
}

namespace SR_GRAPH_NS {
    class Environment;
    class RenderScene;
    class RenderContext;
}

namespace SR_GTYPES_NS {
    class Shader;
}

namespace SR_GTYPES_NS {
    class Material;

    class Mesh : public SR_UTILS_NS::IResource, public Memory::IGraphicsResource {
        friend class Material;
    public:
        using RenderScenePtr = SR_HTYPES_NS::SafePtr<RenderScene>;
    protected:
        explicit Mesh(MeshType type);
        ~Mesh() override;

    public:
        static std::vector<Mesh*> Load(SR_UTILS_NS::Path path, MeshType type);
        static Mesh* TryLoad(SR_UTILS_NS::Path path, MeshType type, uint32_t id);
        static Mesh* Load(SR_UTILS_NS::Path path, MeshType type, uint32_t id);

    public:
        bool Destroy() override;

    protected:
        virtual bool Calculate();

    public:
        IResource* CopyResource(IResource* destination) const override;

        virtual void Draw() = 0;

        virtual void UseMaterial();
        virtual void UseModelMatrix() { }
        virtual void UseSamplers();

        virtual void FreeVideoMemory() override;

    public:
        SR_NODISCARD virtual int32_t GetIBO() { return SR_ID_INVALID; }
        SR_NODISCARD virtual int32_t GetVBO() { return SR_ID_INVALID; }

        SR_NODISCARD virtual int32_t GetIBO() const { return SR_ID_INVALID; }
        SR_NODISCARD virtual int32_t GetVBO() const { return SR_ID_INVALID; }

        SR_NODISCARD virtual bool IsCanCalculate() const;

        SR_NODISCARD virtual SR_FORCE_INLINE bool IsMeshActive() const noexcept { return true; }
        SR_NODISCARD virtual SR_FORCE_INLINE bool IsDebugMesh() const noexcept { return false; }
        SR_NODISCARD Shader* GetShader() const;
        SR_NODISCARD virtual std::string GetGeometryName() const { return std::string(); }
        SR_NODISCARD Material* GetMaterial() const { return m_material; }
        SR_NODISCARD virtual const SR_MATH_NS::Matrix4x4& GetModelMatrix() const;
        SR_NODISCARD int32_t GetVirtualUBO() const { return m_virtualUBO; }
        SR_NODISCARD virtual SR_MATH_NS::FVector3 GetTranslation() const { return SR_MATH_NS::FVector3::Zero(); }

        virtual void SetGeometryName(const std::string& name) { }
        void SetMaterial(Material* material);
        void SetMaterial(const SR_UTILS_NS::Path& path);

        SR_NODISCARD virtual std::vector<uint32_t> GetIndices() const { return { }; }

        virtual void BindMesh() const;

    protected:
        SR_NODISCARD uint64_t GetFileHash() const override { return 0; }
        void OnResourceUpdated(SR_UTILS_NS::ResourceContainer* pContainer, int32_t depth) override;

    protected:
        Memory::UBOManager&          m_uboManager;

        const MeshType               m_type              = MeshType::Unknown;

        Material*                    m_material          = nullptr;
        std::atomic<bool>            m_hasErrors         = false;

        std::atomic<bool>            m_dirtyMaterial     = false;

        int32_t                      m_virtualUBO        = SR_ID_INVALID;

    };
}

#endif //GAMEENGINE_MESH_H
