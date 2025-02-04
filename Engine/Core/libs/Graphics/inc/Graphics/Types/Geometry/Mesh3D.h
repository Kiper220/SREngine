//
// Created by Nikita on 01.06.2021.
//

#ifndef GAMEENGINE_MESH3D_H
#define GAMEENGINE_MESH3D_H

#include <Graphics/Types/Geometry/MeshComponent.h>

namespace SR_GRAPH_NS::Memory {
    class MeshAllocator;
}

namespace SR_GTYPES_NS {
    class Mesh3D final : public MeshComponent {
        friend class Mesh;
        using Super = MeshComponent;
        SR_ENTITY_SET_VERSION(1001);
        SR_INITIALIZE_COMPONENT(Mesh3D);
    public:
        Mesh3D();

    private:
        ~Mesh3D() override;

    public:
        typedef Vertices::StaticMeshVertex VertexType;

    public:
        IResource* CopyResource(IResource* destination) const override;

        static Component* LoadComponent(SR_HTYPES_NS::Marshal& marshal, const SR_HTYPES_NS::DataStorage* dataStorage);

        void UseMaterial() override;
        void UseModelMatrix() override;

        SR_NODISCARD bool IsCanCalculate() const override;
        SR_NODISCARD uint32_t GetMeshId() const { return m_meshId; }

    private:
        void SetRawMesh(SR_HTYPES_NS::RawMesh* raw);

        bool Calculate() override;
        void FreeVideoMemory() override;
        void Draw() override;

        SR_NODISCARD SR_HTYPES_NS::Marshal::Ptr Save(SR_HTYPES_NS::Marshal::Ptr pMarshal, SR_UTILS_NS::SavableFlags flags) const override;
        SR_NODISCARD std::vector<uint32_t> GetIndices() const override;

    protected:
        bool Reload() override;
        bool Load() override;
        bool Unload() override;

    private:
        SR_HTYPES_NS::RawMesh* m_rawMesh = nullptr;
        /// определяет порядок меша в файле, если их там несколько
        int32_t m_meshId = SR_UINT32_MAX;

    };
}

#endif //GAMEENGINE_MESH3D_H
