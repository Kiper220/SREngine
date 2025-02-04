//
// Created by Monika on 05.07.2022.
//

#include <Core/GUI/SceneRunner.h>
#include <Utils/TaskManager/TaskManager.h>

namespace SR_CORE_NS::GUI {
    SceneRunner::SceneRunner()
        : SR_GRAPH_NS::GUI::Widget("Scene runner", SR_MATH_NS::IVector2(0, 50))
    { }

    void SceneRunner::SetScene(const SR_WORLD_NS::Scene::Ptr &scene) {
        SR_LOCK_GUARD
        m_scene = scene;
    }

    void SceneRunner::Draw() {
        auto&& engine = Engine::Instance();

        auto&& font = SR_GRAPH_NS::Environment::Get()->GetIconFont();
        float_t scale = font->Scale;
        font->Scale /= 3;

        const ImVec2 buttonSize = ImVec2(25, 25);

        bool locked = false;

        if (m_scene.TryRecursiveLockIfValid()) {
            m_isActive = engine.IsActive();
            m_isPaused = engine.IsPaused();
            m_lastPath = std::move(m_scene->GetPath());
            locked = true;
        }

        bool active = m_isActive;
        bool paused = m_isPaused;

        {
            ImGui::PushFont(font);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

            ImGui::Separator();

            if (ImGui::Button(active ? SR_ICON_STOP : SR_ICON_PLAY, buttonSize) && locked) {
                active = !active;

                if (active) {
                    active = PlayScene();
                }
                else {
                    paused = false;
                    ReturnScene();
                }
            }

            ImGui::SameLine();

            if (ImGui::Button(paused ? SR_ICON_PAUSE_CIRCLE : SR_ICON_PAUSE, buttonSize)) {
                paused = !paused;
            }

            ImGui::SameLine();

            if (ImGui::Button(SR_ICON_UNDO, buttonSize) && locked) {
                m_scene->Reload();
            }

            ImGui::Separator();

            ImGui::PopFont();
            ImGui::PopStyleVar();

            font->Scale = scale;

            ImGui::Text("%s", m_isActive ? m_scenePath.CStr() : m_lastPath.CStr());

            ImGui::Separator();
        }

        if (locked) {
            engine.SetActive((m_isActive = active));
            engine.SetPaused((m_isPaused = paused));
            m_scene.Unlock();
        }
    }

    bool SceneRunner::PlayScene() {
        SR_LOCK_GUARD

        SR_LOG("SceneRunner::PlayScene() : play scene \"" + m_lastPath.ToString() + "\"");

        if (!m_scene->Save()) {
            SR_ERROR("SceneRunner::PlayScene() : failed to save scene!");
            return false;
        }

        m_scenePath = m_lastPath;

        auto&& runtimePath = SR_UTILS_NS::ResourceManager::Instance().GetCachePath().Concat("Scenes/Runtime-scene.scene");

        if (runtimePath.Exists(SR_UTILS_NS::Path::Type::Folder)) {
            if (!SR_PLATFORM_NS::Delete(runtimePath)) {
                SR_ERROR("SceneRunner::PlayScene() : failed to delete cached scene!");
                return false;
            }
        }

        SR_LOG("SceneRunner::PlayScene() : copy scene: \n\tFrom: " + m_scene->GetPath().ToString() + "\n\tTo: " + runtimePath.ToString());

        if (!m_scene->GetPath().GetFolder().Copy(runtimePath)) {
            SR_ERROR("SceneRunner::PlayScene() : failed to copy scene!\n\tSource: "
                + m_scene->GetPath().ToString() + "\n\tDestination: " + runtimePath.ToString());
            return false;
        }

        auto&& runtimeScene = SR_WORLD_NS::Scene::Load(runtimePath);

        return Engine::Instance().SetScene(runtimeScene);
    }

    void SceneRunner::ReturnScene() {
        SR_LOG("SceneRunner::ReturnScene() : stop scene \"" + m_lastPath.ToString() + "\"");

        auto&& originalScene = SR_WORLD_NS::Scene::Load(m_scenePath);
        Engine::Instance().SetScene(originalScene);
    }
}