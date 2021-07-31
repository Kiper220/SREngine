//
// Created by Nikita on 18.11.2020.
//

#ifndef GAMEENGINE_WINDOW_H
#define GAMEENGINE_WINDOW_H

#include <Types/WindowFormat.h>

#include <Render/Render.h>
#include <Render/Camera.h>
#include <Environment/Environment.h>
#include <Render/PostProcessing.h>
#include <thread>
#include <mutex>
#include <imgui.h> // TODO: remove
#include <imgui_internal.h> // TODO: remove
#include <functional>
#include <Types/EditorGrid.h>
#include <Render/ColorBuffer.h>
#include <Types/Time.h>
#include <Math/Vector3.h>

namespace Framework::Graphics {
    class Window {
    public:
        Window(
                const char* win_name,
                const char* icoPath,
                WindowFormat format,
                Render* render,
                bool vsync,
                bool fullScreen,
                bool resizable,
                unsigned int smoothSamples
                )
                : m_env(Environment::Get())
        {
            this->m_env->InitWindowFormat(format);

            this->m_win_name        = win_name;
            this->m_icoPath         = icoPath;
            this->m_render          = render;
            this->m_fullScreen      = fullScreen;
            this->m_vsync           = vsync;
            this->m_smoothSamples   = smoothSamples;
            this->m_resizable       = resizable;
        }
    private:
        ~Window() = default;
    private:
        volatile bool         m_isCreate              = false;
        volatile bool         m_isInit                = false;
        volatile bool         m_isRun                 = false;
        volatile bool         m_isClose               = false;

        volatile bool         m_hasErrors             = false;
        volatile bool         m_isEnvInit             = false;
        volatile bool         m_isWindowClose         = false;

        volatile bool         m_isWindowFocus         = true;

        volatile bool         m_isNeedResize          = false;
        volatile bool         m_isNeedMove            = false;
    private:
        std::thread           m_thread                = std::thread();

        Helper::Types::Time*  m_time                  = nullptr;

        Environment*          m_env                   = nullptr;

        const char*           m_win_name              = "Unnamed";
        const char*           m_icoPath               = "Unknown";
        uint8_t               m_smoothSamples         = 4;

        Render*               m_render                = nullptr;

        std::mutex            m_camerasMutex          = std::mutex();
        std::vector<Camera*>  m_newCameras            = std::vector<Camera*>();
        uint32_t              m_countNewCameras       = 0;
        std::vector<Camera*>  m_camerasToDestroy      = std::vector<Camera*>();
        uint32_t              m_countCamerasToDestroy = 0;
        std::vector<Camera*>  m_cameras               = std::vector<Camera*>();
        uint32_t              m_countCameras          = 0;

        /* 1 - current, 2 - new */
        std::pair<bool, bool> m_GUIEnabled            = { false, false };

        ImGuiWindow*          m_aimedWindowTarget     = nullptr;
        Camera*               m_aimedCameraTarget     = nullptr;
        Mesh*                 m_aimedMesh             = nullptr;
        bool                  m_requireGetAimed       = false;

        GUI::ICanvas*         m_canvas                = nullptr;

        bool                  m_vsync                 = false;
        bool                  m_fullScreen            = false;
        bool                  m_resizable             = false;

        Math::Vector2         m_windowPos             = { 0, 0 };
        Math::Vector2         m_newWindowPos          = { 0, 0 };
        Math::Vector2         m_newWindowSize         = { 0, 0 };
    private:
        void PollEvents();
        void Thread();
        bool InitEnvironment();

        void DrawToCamera(Framework::Graphics::Camera* camera);
    public:
        void AddCamera(Camera* camera);
        void DestroyCamera(Camera* camera);

        [[nodiscard]] SR_FORCE_INLINE Render* GetRender() {
            if (!m_render) {
                Debug::Error("Window::GetRender() : render is nullptr! Engine may be crash...");
                return nullptr;
            }
            return m_render;
        }
        [[nodiscard]] inline bool IsRun() const noexcept { return m_isRun; }
        [[nodiscard]] inline bool IsGUIEnabled() const noexcept { return m_GUIEnabled.first; }
        [[nodiscard]] Mesh* PopAimedMesh() noexcept;
        [[nodiscard]] bool RequireAimedMesh(Camera* camera, ImGuiWindow* window) noexcept;
        glm::vec2 GetGlobalWindowMousePos(Camera* camera, ImGuiWindow* win);
    public:
        void CentralizeWindow();
        void Resize(uint32_t w, uint32_t h);
        void CentralizeCursor() noexcept;

        SR_FORCE_INLINE void SetGUIEnabled(bool value) noexcept {
            if (value)
                Helper::Debug::Log("Window::SetGUIEnabled() : enable gui...");
            else
                Helper::Debug::Log("Window::SetGUIEnabled() : disable gui...");

            m_env->SetBuildState(false);
            this->m_env->SetGUIEnabled(value);
            this->m_GUIEnabled.second = value;
        }
    public:
        [[nodiscard]] SR_FORCE_INLINE bool IsFullScreen()           const noexcept { return this->m_env->IsFullScreen(); }
        SR_FORCE_INLINE void SetFullScreen(bool value)              const noexcept { this->m_env->SetFullScreen(value);  }
        [[nodiscard]] SR_FORCE_INLINE bool IsWindowOpen()           const noexcept { return !this->m_isWindowClose;      }
        [[nodiscard]] SR_FORCE_INLINE bool IsWindowFocus()          const noexcept { return this->m_isWindowFocus;       }
        [[nodiscard]] SR_FORCE_INLINE Math::Vector2 GetWindowSize() const noexcept { return m_env->GetWindowSize();      }
    public:
        bool Create(GUI::ICanvas* canvas);
        bool Init();
        bool Run();
        bool Close();
        bool Free();
    };
}

#endif //GAMEENGINE_WINDOW_H
