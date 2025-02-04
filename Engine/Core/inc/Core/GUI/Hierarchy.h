//
// Created by Monika on 11.02.2022.
//

#ifndef SRENGINE_HIERARCHY_H
#define SRENGINE_HIERARCHY_H

#include <Utils/Types/SafePointer.h>
#include <Utils/World/Scene.h>
#include <Utils/Input/InputEvents.h>
#include <Utils/ECS/GameObject.h>

#include <Graphics/GUI/Widget.h>

namespace SR_CORE_NS::GUI {
    class Hierarchy : public SR_GRAPH_NS::GUI::Widget {
    public:
        Hierarchy();
        ~Hierarchy() override;

    public:
        void Update();

        void SetScene(const SR_WORLD_NS::Scene::Ptr& scene) override;

        void OnKeyDown(const SR_UTILS_NS::KeyboardInputData* data) override;
        void OnKeyUp(const SR_UTILS_NS::KeyboardInputData* data) override;

        SR_NODISCARD std::set<SR_UTILS_NS::GameObject::Ptr> GetSelected() const;

        void ClearSelected();
        void SelectGameObject(const SR_UTILS_NS::GameObject::Ptr& ptr);

        void SetSelectedImpl(const std::set<SR_UTILS_NS::GameObject::Ptr>& changeSelected);

    private:
        void Draw() override;
        void CheckSelected(const SR_UTILS_NS::GameObject::Ptr& gm);
        void ContextMenu(const SR_UTILS_NS::GameObject::Ptr& gm, uint64_t id);
        void DrawChild(const SR_UTILS_NS::GameObject::Ptr& root);
        void Copy() const;
        void Paste();
        void Delete();

    private:
        const ImGuiTreeNodeFlags m_nodeFlagsWithChild = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        const ImGuiTreeNodeFlags m_nodeFlagsWithoutChild = ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf;

        SR_WORLD_NS::Scene::Ptr m_scene;
        std::list<SR_UTILS_NS::GameObject::Ptr> m_pointersHolder;
        std::set<SR_UTILS_NS::GameObject::Ptr> m_selected;
        SR_UTILS_NS::GameObject::GameObjects m_tree;

        std::atomic<bool> m_shiftPressed;

        SR_GRAPH_NS::GUI::Widget* m_sceneRunnerWidget = nullptr;

    };
}

#endif //SRENGINE_HIERARCHY_H
