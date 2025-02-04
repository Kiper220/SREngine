//
// Created by Nikita on 27.11.2020.
//

#ifndef GAMEENGINE_GAMEOBJECT_H
#define GAMEENGINE_GAMEOBJECT_H

#include <Utils/ECS/EntityManager.h>
#include <Utils/ECS/IComponentable.h>
#include <Utils/ECS/TagManager.h>

#include <Utils/Math/Vector3.h>
#include <Utils/Types/SafePointer.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Types/SafeVariable.h>

namespace SR_UTILS_NS::World {
    class Scene;
}

namespace SR_UTILS_NS {
    class Transform3D;
    class Transform;
    class Component;

    SR_ENUM_NS(GameObjectDestroyBy,
        GAMEOBJECT_DESTROY_BY_UNKNOWN    = 1 << 0,
        GAMEOBJECT_DESTROY_BY_SCENE      = 1 << 1,
        GAMEOBJECT_DESTROY_BY_GAMEOBJECT = 1 << 2,
        GAMEOBJECT_DESTROY_BY_OTHER      = 1 << 3,
        GAMEOBJECT_DESTROY_BY_COMMAND    = 1 << 4
    );
    typedef uint64_t GODestroyByBits;

    SR_ENUM_NS(GameObjectFlags,
        GAMEOBJECT_FLAG_NONE    = 1 << 0,
        GAMEOBJECT_FLAG_NO_SAVE = 1 << 1
    );
    typedef uint64_t GameObjectFlagBits;

    class SR_DLL_EXPORT GameObject : public SR_HTYPES_NS::SharedPtr<GameObject>, public IComponentable, public Entity {
        SR_ENTITY_SET_VERSION(1005);
        friend class Component;
    public:
        using Name = std::string;
        using Ptr = SR_HTYPES_NS::SharedPtr<GameObject>;
        using Super = Ptr;
        using GameObjects = std::vector<GameObject::Ptr>;
        using ScenePtr = World::Scene*;
        using IdGetterFn = SR_HTYPES_NS::Function<uint64_t(const GameObject::Ptr&)>;

    public:
        GameObject(std::string name, Transform* pTransform);
        explicit GameObject(std::string name);

        ~GameObject() override;

        static GameObject::Ptr Load(SR_HTYPES_NS::Marshal& marshal, const ScenePtr& scene);

    public:
        SR_NODISCARD GameObject::Ptr Copy(const ScenePtr& scene) const;

        SR_NODISCARD ScenePtr GetScene() const { return m_scene; }
        SR_NODISCARD Transform* GetParentTransform() const noexcept { return m_parent ? m_parent->m_transform : nullptr; }
        SR_NODISCARD Transform* GetTransform() const noexcept { return m_transform; }
        SR_NODISCARD GameObject::Ptr GetParent() const noexcept { return m_parent; }
        SR_NODISCARD GameObject::Ptr Find(uint64_t hashName) const noexcept;
        SR_NODISCARD std::string GetName() const { return m_name; }
        SR_NODISCARD Tag GetTag() const;
        SR_NODISCARD bool HasTag() const;
        SR_NODISCARD bool IsActive() const noexcept;
        SR_NODISCARD SR_FORCE_INLINE bool IsEnabled() const noexcept { return m_isEnabled; }
        SR_NODISCARD SR_FORCE_INLINE uint64_t GetHashName() const noexcept { return m_hashName; }
        SR_NODISCARD SR_FORCE_INLINE uint64_t GetIdInScene() const noexcept { return m_idInScene; }
        SR_NODISCARD SR_INLINE bool HasChildren() const { return !m_children.empty(); }
        SR_NODISCARD SR_INLINE GameObjects& GetChildrenRef() { return m_children; }
        SR_NODISCARD SR_INLINE const GameObjects& GetChildrenRef() const { return m_children; }
        SR_NODISCARD SR_INLINE GameObjects GetChildren() const { return m_children; }
        SR_NODISCARD SR_INLINE GameObjectFlagBits GetFlags() const { return m_flags; }

        SR_NODISCARD std::string GetEntityInfo() const override;

        SR_NODISCARD SR_HTYPES_NS::Marshal::Ptr Save(SR_HTYPES_NS::Marshal::Ptr pMarshal, SavableFlags flags) const override;
        SR_NODISCARD std::list<EntityBranch> GetEntityBranches() const override;

        SR_MATH_NS::FVector3 GetBarycenter();
        SR_MATH_NS::FVector3 GetHierarchyBarycenter();

        void SetIdInScene(uint64_t id);
        void SetScene(ScenePtr pScene);

        void ForEachChild(const std::function<void(GameObject::Ptr&)>& fun);
        void ForEachChild(const std::function<void(const GameObject::Ptr&)>& fun) const;
        bool SetParent(const GameObject::Ptr& parent);
        void SetName(std::string name);
        void SetTag(const std::string& tag);

        bool Contains(const GameObject::Ptr& child);
        void SetEnabled(bool value);
        void Destroy(GODestroyByBits by = GAMEOBJECT_DESTROY_BY_OTHER);
        void SetTransform(Transform* transform);
        void SetFlags(GameObjectFlagBits flags) { m_flags = flags; }

        bool MoveToTree(const GameObject::Ptr& destination);
        bool AddChild(const GameObject::Ptr& child);
        void RemoveChild(const GameObject::Ptr& child);

        /// Вызывает OnAttached у компонентов загруженных через LoadComponent
        bool PostLoad() override;

        void Awake(bool isPaused) noexcept override;
        void Start() noexcept override;

        void CheckActivity() noexcept override;

        void SetDirty(bool value) override;
        void OnMatrixDirty();

    private:
        void OnAttached();

        bool UpdateEntityPath();

    private:
        bool m_isEnabled = true;
        bool m_isActive = false;
        bool m_isDestroy = false;

        uint64_t m_hashName = 0;
        uint64_t m_idInScene = SR_ID_INVALID;

        GameObject::Ptr m_parent;
        GameObjects m_children;

        ScenePtr m_scene = nullptr;
        Transform* m_transform  = nullptr;

        Name m_name;
        Tag m_tag = 0;

        GameObjectFlagBits m_flags = GAMEOBJECT_FLAG_NONE;

    };
}

#endif //GAMEENGINE_GAMEOBJECT_H
