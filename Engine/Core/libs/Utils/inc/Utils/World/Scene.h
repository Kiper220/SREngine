//
// Created by Nikita on 30.11.2020.
//

#ifndef SRENGINE_SCENE_H
#define SRENGINE_SCENE_H

#include <Utils/ECS/IComponentable.h>
#include <Utils/Types/SafePointer.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/World/Observer.h>
#include <Utils/Types/StringAtom.h>
#include <Utils/Types/Marshal.h>
#include <Utils/World/CameraData.h>
#include <Utils/Types/DataStorage.h>
#include <Utils/World/TensorKey.h>

namespace SR_UTILS_NS {
    class GameObject;
}

namespace SR_HTYPES_NS {
    class RawMesh;
}

namespace SR_WORLD_NS {
    class SceneLogic;

    class SR_DLL_EXPORT Scene : public SR_HTYPES_NS::SafePtr<Scene>, public SR_UTILS_NS::IComponentable {
    public:
        using Ptr = SR_HTYPES_NS::SafePtr<Scene>;
        using Super = Ptr;
        using GameObjectPtr = SR_HTYPES_NS::SharedPtr<GameObject>;
        using GameObjects = std::vector<GameObjectPtr>;

        ~Scene() override;

    protected:
        Scene();

    public:
        static Scene::Ptr New(const Path& path);
        static Scene::Ptr Load(const Path& path);

        bool Save();
        bool SaveAt(const Path& path);
        bool Destroy();
        void Update(float_t dt);

    public:
        template<typename T> SR_NODISCARD T* GetLogic() const {
            return dynamic_cast<T*>(m_logic);
        }

        void SetPath(const Path& path) { m_path = path; }

        SR_NODISCARD std::string GetName() const;
        SR_NODISCARD Path GetPath() const { return m_path; }
        SR_NODISCARD SR_HTYPES_NS::DataStorage& GetDataStorage() { return m_dataStorage; }
        SR_NODISCARD const SR_HTYPES_NS::DataStorage& GetDataStorage() const { return m_dataStorage; }

        GameObjects& GetRootGameObjects();

        GameObjectPtr FindByComponent(const std::string& name);
        GameObjectPtr Find(const std::string& name);
        GameObjectPtr Find(uint64_t hashName);

        void RegisterGameObject(const GameObjectPtr& ptr);

        virtual GameObjectPtr InstanceFromFile(const std::string& path);
        virtual GameObjectPtr FindOrInstance(const std::string& name);
        virtual GameObjectPtr Instance(const std::string& name);
        virtual GameObjectPtr Instance(const Types::RawMesh* rawMesh);
        virtual GameObjectPtr Instance(SR_HTYPES_NS::Marshal& marshal);

    public:
        bool Remove(const GameObjectPtr& gameObject);

        void OnChanged();

        bool Reload();

    private:
        SceneLogic* m_logic = nullptr;

        bool m_isPrefab = false;
        bool m_isDestroy = false;

        std::atomic<bool>  m_isHierarchyChanged = false;

        Path m_path;

        SR_HTYPES_NS::DataStorage m_dataStorage;

        std::list<uint64_t> m_freeObjIndices;

        GameObjects m_gameObjects;
        GameObjects m_rootObjects;

    };
}

#endif //SRENGINE_SCENE_H
