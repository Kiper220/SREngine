//
// Created by Nikita on 19.11.2020.
//

#ifndef GAMEENGINE_POSTPROCESSING_H
#define GAMEENGINE_POSTPROCESSING_H

#include "Shader.h"
#include <Environment/Environment.h>
#include <Debug.h>

namespace Framework::Graphics {
    class Render;
    class Camera;
    /*
     How use?

     void Draw() {
        ...clear buffers

        postProcessing->Begin();

        ...draw geometry

        postProcessing->End();

        ...swapBuffers
     }
     */
    // TODO: add freeing video buffers!
    class PostProcessing {
        /** \brief vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates. */
    private:
        ~PostProcessing() = default;
    public:
        PostProcessing(Camera* camera);
    private:
        float			          m_gamma					    = 0.8f;
        float                     m_exposure                    = 1.f;
        glm::vec3		          m_color_correction		    = { 1, 1, 1 };
        float                     m_bloomIntensity              = 1.f;
        volatile unsigned int     m_bloomAmount                 = 10;
    private:
        bool                      m_debugDisplayBloomMask       = false;
    private:
        //unsigned int	m_screen_pp_texture		                = 0;
        //unsigned int	m_pp_FBO				                = 0;
        //unsigned int	m_pp_RBO				                = 0;

        bool                      m_horizontal                  = false;
        bool                      m_firstIteration              = false;

        unsigned int              m_VAO                         = 0;
        unsigned int              m_VBO                         = 0;

        volatile bool             m_bloom                       = true;

        unsigned int              m_skyboxRBO                   = 0;
        unsigned int              m_skyboxFBO                   = 0;
        unsigned int              m_skyboxColorBuffer           = 0;

        unsigned int              m_finalRBO                    = 0;
        unsigned int              m_finalFBO                    = 0;
        unsigned int              m_finalColorBuffer            = 0;

        unsigned int              m_RBODepth                    = 0;
        unsigned int              m_HDRFrameBufferObject        = 0;
        std::vector<unsigned int> m_ColorBuffers                = { 0, 0 };

        std::vector<unsigned int> m_PingPongFrameBuffers        = { 0, 0 };
        std::vector<unsigned int> m_PingPongColorBuffers        = { 0, 0 };
    private:
        void BlurBloom();
    private:
        Environment*    m_env                                   = nullptr;

        Shader*         m_postProcessingShader                  = nullptr;
        Shader*         m_blurShader                            = nullptr;

        Camera*         m_camera                                = nullptr;
        Render*         m_render                                = nullptr;
        bool            m_isInit                                = false;
    public:
        [[nodiscard]] inline glm::vec3 GetColorCorrection() const noexcept { return m_color_correction; }
        [[nodiscard]] inline float GetGamma() const noexcept { return m_gamma; }
    public:
        inline void SetGamma(float gamma) noexcept { m_gamma = gamma; }
        inline void SetExposure(float exposure) noexcept { m_exposure = exposure; }
        inline void SetBloomAmount(unsigned int amount) noexcept { this->m_bloomAmount = amount; }
        inline void SetBloomIntensity(float intensity) noexcept { this->m_bloomIntensity = intensity; }

        inline void SetBloom(bool v) { this->m_bloom = v; }
        inline void SetDisplayBloomMask(bool value) noexcept { m_debugDisplayBloomMask = value; }
    public:
        /** \brief Init shader and set default values \warning Call only from window context \return bool */
        bool Init(Render* render);
        bool Destroy();

        inline bool Free() noexcept {
            Helper::Debug::Graph("PostProcessing::Free() : free post processing pointer...");
            delete this;
            return true;
        }

        bool ReCalcFrameBuffers(int w, int h);

        void BeginSkybox();
        void EndSkybox();

        bool Begin();
        bool End();

        [[nodiscard]] inline unsigned int GetFinally() const noexcept {
            return this->m_finalColorBuffer;
        }
        [[nodiscard]] inline unsigned int GetColoredImage() const noexcept {
            return this->m_ColorBuffers[0];
        }
        [[nodiscard]] inline unsigned int GetBloomMask() const noexcept {
            return this->m_ColorBuffers[1];
        }
        [[nodiscard]] inline unsigned int GetBlurBloomMask() const noexcept {
            return m_PingPongColorBuffers[0];
        }
    };
}

#endif //GAMEENGINE_POSTPROCESSING_H
