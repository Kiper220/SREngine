//
// Created by Monika on 06.05.2022.
//

#include <Graphics/Types/Framebuffer.h>
#include <Graphics/Pipeline/Environment.h>
#include <Graphics/Types/Shader.h>

namespace SR_GTYPES_NS {
    Framebuffer::Framebuffer()
        : Super(SR_COMPILE_TIME_CRC32_TYPE_NAME(Framebuffer), true /** auto remove */)
        , m_pipeline(Environment::Get())
    {
        SR_UTILS_NS::ResourceManager::Instance().RegisterResource(this);
    }

    Framebuffer::~Framebuffer() {
        SRAssert(m_frameBuffer == SR_ID_INVALID);

    #ifdef SR_DEBUG
        for (auto&& [texture, format] : m_colors) {
            SRAssert(texture == SR_ID_INVALID);
            SR_UNUSED_VARIABLE(format);
        }
    #endif

        SRAssert(m_depth.texture == SR_ID_INVALID);
    }

    Framebuffer::Ptr Framebuffer::Create(uint32_t images, const SR_MATH_NS::IVector2 &size) {
        std::list<ColorFormat> colors;

        for (uint32_t i = 0; i < images; ++i) {
            colors.emplace_back(ColorFormat::RGBA8_UNORM);
        }

        return Create(colors, DepthFormat::Auto, size);
    }

    Framebuffer::Ptr Framebuffer::Create(const std::list<ColorFormat> &colors, DepthFormat depth, const SR_MATH_NS::IVector2 &size) {
        return Create(colors, depth, size, 0);
    }

    Framebuffer::Ptr Framebuffer::Create(const std::list<ColorFormat> &colors, DepthFormat depth, const SR_MATH_NS::IVector2 &size, uint8_t samples) {
        Framebuffer* fbo = new Framebuffer();

        SRAssert(!size.HasZero() && !size.HasNegative());

        fbo->SetSize(size);
        fbo->m_depth.format = depth;
        fbo->m_sampleCount = samples;

        for (auto&& color : colors) {
            ColorLayer layer;
            layer.format = color;
            fbo->m_colors.emplace_back(layer);
        }

        return fbo;
    }

    Framebuffer::Ptr Framebuffer::Create(const std::list<ColorFormat> &colors, DepthFormat depth) {
        return Create(colors, depth, SR_MATH_NS::IVector2(0, 0));
    }

    bool Framebuffer::Bind() {
        if (m_hasErrors) {
            return false;
        }

        if ((!IsCalculated() || m_dirty) && !Init()) {
            SR_ERROR("Framebuffer::Bind() : failed to initialize framebuffer!");
            return false;
        }

        m_pipeline->BindFrameBuffer(m_frameBuffer);
        m_pipeline->SetCurrentFramebuffer(this);

        return true;
    }

    bool Framebuffer::Init() {
        if (!OnResize()) {
            SR_ERROR("Framebuffer::OnResize() : failed to resize frame buffer!");
            return false;
        }

        m_isCalculated = true;

        return true;
    }

    void Framebuffer::FreeVideoMemory() {
        if (m_frameBuffer != SR_ID_INVALID) {
            SRVerifyFalse(!m_pipeline->FreeFBO(m_frameBuffer));
            m_frameBuffer = SR_ID_INVALID;
        }

        if (m_depth.texture != SR_ID_INVALID) {
            SRVerifyFalse(!m_pipeline->FreeTexture(&m_depth.texture));
        }

        for (auto&& [texture, format] : m_colors) {
            if (texture == SR_ID_INVALID) {
                continue;
            }

            SRVerifyFalse(!m_pipeline->FreeTexture(&texture));
        }

        IGraphicsResource::FreeVideoMemory();
    }

    bool Framebuffer::OnResize() {
        if (m_size.HasZero() || m_size.HasNegative()) {
            SR_ERROR("Framebuffer::OnResize() : incorrect FBO size!");
            m_hasErrors = true;
            return false;
        }

        if (!m_pipeline->CreateFrameBuffer(
            m_size.ToGLM(),
            m_frameBuffer,
            m_depthEnabled ? &m_depth : nullptr,
            m_colors,
            m_sampleCount)
        ) {
            SR_ERROR("Framebuffer::OnResize() : failed to create frame buffer!");
            m_hasErrors = true;
            return false;
        }

        m_hasErrors = false;
        m_dirty = false;

        return true;
    }

    void Framebuffer::SetSize(const SR_MATH_NS::IVector2 &size) {
        m_size = size;
        m_dirty = true;
    }

    bool Framebuffer::BeginRender(const Framebuffer::ClearColors &clearColors, float_t depth) {
        m_pipeline->ClearBuffers(clearColors, depth);

        if (!m_pipeline->BeginRender()) {
            return false;
        }

        m_pipeline->SetViewport(m_size.x, m_size.y);
        m_pipeline->SetScissor(m_size.x, m_size.y);

        return true;
    }

    bool Framebuffer::BeginRender() {
        m_pipeline->ClearBuffers();

        if (!m_pipeline->BeginRender()) {
            return false;
        }

        m_pipeline->SetViewport(m_size.x, m_size.y);
        m_pipeline->SetScissor(m_size.x, m_size.y);

        return true;
    }

    void Framebuffer::EndRender() {
        m_pipeline->EndRender();
    }

    int32_t Framebuffer::GetId() {
        if (m_hasErrors) {
            return SR_ID_INVALID;
        }

        if ((!IsCalculated() || m_dirty) && !Init()) {
            SR_ERROR("Framebuffer::GetId() : failed to initialize framebuffer!");
        }

        return m_frameBuffer;
    }

    uint64_t Framebuffer::GetFileHash() const {
        return 0;
    }

    int32_t Framebuffer::GetColorTexture(uint32_t layer) const {
        if (layer >= m_colors.size() || m_hasErrors || m_dirty) {
            return SR_ID_INVALID;
        }

        return m_colors.at(layer).texture;
    }

    bool Framebuffer::BeginRender(const SR_MATH_NS::FColor &clearColor, float_t depth) {
        return BeginRender(Framebuffer::ClearColors{ clearColor }, depth);
    }

    uint32_t Framebuffer::GetWidth() const {
        return m_size.x;
    }

    uint32_t Framebuffer::GetHeight() const {
        return m_size.y;
    }

    void Framebuffer::SetDepthEnabled(bool depthEnabled) {
        m_depthEnabled = depthEnabled;
        m_dirty = true;
    }

    void Framebuffer::SetSampleCount(uint8_t samples) {
        m_sampleCount = samples;
        m_dirty = true;
    }

    int32_t Framebuffer::GetDepthTexture() const {
        if (!m_depthEnabled) {
            return SR_ID_INVALID;
        }

        return m_depth.texture;
    }
}