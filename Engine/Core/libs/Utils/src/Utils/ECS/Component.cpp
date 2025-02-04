//
// Created by Nikita on 27.11.2020.
//

#include <Utils/ECS/Component.h>
#include <Utils/ECS/GameObject.h>
#include <Utils/ECS/Transform2D.h>
#include <Utils/ECS/ComponentManager.h>
#include <Utils/Types/Thread.h>
#include <Utils/World/Scene.h>

namespace SR_UTILS_NS {
    SR_HTYPES_NS::Marshal::Ptr Component::Save(SR_HTYPES_NS::Marshal::Ptr pMarshal, SavableFlags flags) const {
        pMarshal = Entity::Save(pMarshal, flags);

        pMarshal->Write<uint64_t>(GetComponentHashName());
        pMarshal->Write(IsEnabled());
        pMarshal->Write<uint16_t>(ComponentManager::Instance().GetVersion(this));

        return pMarshal;
    }

    void Component::SetParent(IComponentable* pParent) {
        m_parent = pParent;
    }

    void Component::SetEnabled(bool value) {
        if (m_isEnabled == value) {
            return;
        }

        m_isEnabled = value;

        if (m_parent) {
            m_parent->SetDirty(true);
        }
    }

    void Component::CheckActivity() {
        auto&& pParent = dynamic_cast<SR_UTILS_NS::GameObject*>(m_parent);

        /// если родителя нет, или он отличается от ожидаемого, то будем считать что родитель активен
        const bool isActive = m_isEnabled && (!pParent || pParent->m_isActive);
        if (isActive == m_isActive) {
            return;
        }

        if ((m_isActive = isActive)) {
            OnEnable();
        }
        else {
            OnDisable();
        }
    }

    Component::ScenePtr Component::GetScene() const {
        if (auto&& pScene = TryGetScene()) {
            return pScene;
        }

        SRHalt("The component have not a valid parent!");

        return nullptr;
    }

    Component::ScenePtr Component::TryGetScene() const {
        /// Игровой объект или сцена никогда не уничтожится до того,
        /// как не установит "m_parent" в "nullptr"

        if (auto&& pGameObject = dynamic_cast<SR_UTILS_NS::GameObject*>(m_parent)) {
            return pGameObject->GetScene();
        }

        if (auto&& pScene = dynamic_cast<SR_WORLD_NS::Scene*>(m_parent)) {
            return pScene;
        }

        return nullptr;
    }

    Component::GameObjectPtr Component::GetGameObject() const {
        if (auto&& pGameObject = dynamic_cast<SR_UTILS_NS::GameObject*>(m_parent)) {
            return pGameObject->GetThis();
        }

        return GameObjectPtr();
    }

    IComponentable* Component::GetParent() const {
        return m_parent;
    }

    GameObject::Ptr Component::GetRoot() const {
        auto&& pParent = dynamic_cast<SR_UTILS_NS::GameObject*>(m_parent);

        if (!pParent) {
            return GameObjectPtr();
        }

        GameObjectPtr root = pParent->GetThis();

        while (root.Valid()) {
            if (auto&& parent = root->GetParent()) {
                root = parent;
            }
            else {
                break;
            }
        }

        return root;
    }

    Transform *Component::GetTransform() const noexcept {
        if (auto&& pGameObject = dynamic_cast<SR_UTILS_NS::GameObject*>(m_parent)) {
            return pGameObject->GetTransform();
        }

        return nullptr;
    }

    std::string Component::GetEntityInfo() const {
        return "Component: " + GetComponentName();
    }

    Component *Component::CopyComponent() const {
        SRHalt("Not implemented!");
        return nullptr;
    }
}

