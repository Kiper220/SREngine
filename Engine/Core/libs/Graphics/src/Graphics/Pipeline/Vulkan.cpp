//
// Created by Nikita on 25.03.2021.
//

#include <Utils/GUI.h>
#include <Utils/Common/Features.h>
#include <Utils/Platform/Platform.h>

#include <Graphics/Pipeline/Vulkan/AbstractCasts.h>
#include <Graphics/Pipeline/Vulkan.h>
#include <Graphics/Memory/MeshManager.h>

#if defined(SR_WIN32)
    #include <vulkan/vulkan_win32.h>
    #include <Graphics/Window/Win32Window.h>
#elif defined(SR_ANDROID)
    #include <Graphics/Window/AndroidWindow.h>
    #include <vulkan/vulkan_android.h>
#endif

namespace Framework::Graphics {
    const std::vector<const char *> Vulkan::m_deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME,
            //VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME
            //VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME,
            //VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME
    };

#define SR_VRAM ("{" + std::to_string(Environment::Get()->GetVRAMUsage() / 1024 / 1024) + "} ")

    void SRVulkan::SetGUIEnabled(bool enabled) {
        if (auto&& vkImgui = dynamic_cast<Framework::Graphics::Vulkan*>(Environment::Get())->GetVkImGUI()) {
            vkImgui->SetEnabled(enabled);
        }
        VulkanKernel::SetGUIEnabled(enabled);
    }

    bool Vulkan::PreInit(
            unsigned int smooth_samples,
            const std::string &appName,
            const std::string &engineName,
            const std::string &glslc)
    {
        EvoVulkan::Tools::VkFunctionsHolder::Instance().LogCallback = [](const std::string &msg) { SR_VULKAN_LOG(SR_VRAM + msg); };
        EvoVulkan::Tools::VkFunctionsHolder::Instance().WarnCallback = [](const std::string &msg) { SR_WARN(SR_VRAM + msg); };
        EvoVulkan::Tools::VkFunctionsHolder::Instance().ErrorCallback = [](const std::string &msg) { SR_VULKAN_ERROR(SR_VRAM + msg); };
        EvoVulkan::Tools::VkFunctionsHolder::Instance().GraphCallback = [](const std::string &msg) { SR_VULKAN_MSG(SR_VRAM + msg); };

        EvoVulkan::Tools::VkFunctionsHolder::Instance().AssertCallback = [](const std::string &msg) {
            SRAssert2(false, SR_VRAM + msg);
            return false;
        };

        EvoVulkan::Tools::VkFunctionsHolder::Instance().CreateFolder = [](const std::string& path) -> bool {
            return SR_PLATFORM_NS::CreateFolder(path);
        };

        EvoVulkan::Tools::VkFunctionsHolder::Instance().Delete = [](const std::string& path) -> bool {
            return SR_PLATFORM_NS::Delete(path);
        };

        EvoVulkan::Tools::VkFunctionsHolder::Instance().IsExists = [](const std::string& path) -> bool {
            return SR_PLATFORM_NS::IsExists(path);
        };

        EvoVulkan::Tools::VkFunctionsHolder::Instance().Copy = [](const std::string& from, const std::string& to) -> bool {
            return SR_PLATFORM_NS::Copy(from, to);
        };

        m_imgui = new VulkanTypes::VkImGUI();

        m_kernel = new SRVulkan();

        SR_INFO("Vulkan::PreInit() : pre-initializing vulkan...");

    #ifdef SR_ANDROID
        m_enableValidationLayers = false;
    #else
        m_enableValidationLayers = SR_UTILS_NS::Features::Instance().Enabled("VulkanValidation", false);
    #endif

        if (m_enableValidationLayers) {
            m_kernel->SetValidationLayersEnabled(true);
        }

        m_viewport = EvoVulkan::Tools::Initializers::Viewport(0, 0, 0, 0);
        m_scissor = EvoVulkan::Tools::Initializers::Rect2D(0, 0, 0, 0);
        m_cmdBufInfo = EvoVulkan::Tools::Initializers::CommandBufferBeginInfo();
        m_renderPassBI = EvoVulkan::Tools::Insert::RenderPassBeginInfo(0, 0, VK_NULL_HANDLE, VK_NULL_HANDLE, nullptr, 0);

        m_kernel->SetMultisampling(smooth_samples);

        /// TODO: вынести в конфиг
        m_kernel->SetSwapchainImagesCount(2);

        std::vector<const char*>&& validationLayers = { };
        std::vector<const char*>&& instanceExtensions = {
                VK_KHR_SURFACE_EXTENSION_NAME,
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
        #ifdef SR_WIN32
                VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        #endif
        #ifdef SR_ANDROID
                VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
        #endif
        };

        if (m_enableValidationLayers) {
            instanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            validationLayers.emplace_back("VK_LAYER_KHRONOS_validation");
        }

        if (!m_kernel->PreInit(
                appName,
                engineName,
                glslc,
                instanceExtensions,
                validationLayers))
        {
            SR_ERROR("Vulkan::PreInit() : failed to pre-init Evo Vulkan kernel!");
            return false;
        }

        return true;
    }

    bool SRVulkan::IsWindowValid() const {
        auto&& pPipeline = dynamic_cast<SR_GRAPH_NS::Vulkan*>(Environment::Get());
        if (!pPipeline) {
            return false;
        }

        return pPipeline->GetWindow().Do<bool>([](Window* pWindow) -> bool {
            return pWindow->IsValid();
        }, false);
    }

    bool Vulkan::OnResize(const Helper::Math::UVector2 &size) {
        m_kernel->SetSize(size.x, size.y);

        return Environment::OnResize(size);
    }

 //  bool Vulkan::CloseWindow() {
 //      SR_GRAPH_LOG("Vulkan::CloseWindow() : close window...");

 //      SR_GRAPH_NS::Memory::MeshManager::Instance().PrintDump();

 //      if (m_memory) {
 //          m_memory->Free();
 //          m_memory = nullptr;
 //      }

 //      if (m_kernel) {
 //          if (!m_kernel->Destroy()) {
 //              SR_ERROR("Vulkan::CloseWindow() : failed to destroy Evo Vulkan kernel!");
 //              return false;
 //          }
 //      }

 //      return true;
 //  }

    bool Vulkan::Init(const WindowPtr& window, int swapInterval) {
        SR_GRAPH_LOG("Vulkan::Init() : initializing vulkan...");

        m_window = window;

        auto createSurf = [window](const VkInstance &instance) -> VkSurfaceKHR {
    #ifdef SR_WIN32 // TODO: use VK_USE_PLATFORM_WIN32_KHR
            if (auto&& pImpl = window->GetImplementation<Win32Window>()) {
                VkWin32SurfaceCreateInfoKHR surfaceInfo = { };
                surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
                surfaceInfo.pNext = nullptr;
                surfaceInfo.flags = 0;
                surfaceInfo.hinstance = pImpl->GetHINSTANCE();
                surfaceInfo.hwnd = pImpl->GetHWND();

                VkSurfaceKHR surface = VK_NULL_HANDLE;
                VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
                if (result != VK_SUCCESS) {
                    return VK_NULL_HANDLE;
                }
                else
                    return surface;
            }
            else {
                SR_ERROR("Vulkan::Init() : window is not support this architecture!");
                return VK_NULL_HANDLE;
            }
    #elif defined(SR_ANDROID)
            if (auto&& pImpl = window->GetImplementation<AndroidWindow>()) {
                VkAndroidSurfaceCreateInfoKHR surfaceInfo = { };
                surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
                surfaceInfo.pNext = nullptr;
                surfaceInfo.flags = 0;
                surfaceInfo.window = pImpl->GetNativeWindow();

                VkSurfaceKHR surface = VK_NULL_HANDLE;
                VkResult result = vkCreateAndroidSurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
                if (result != VK_SUCCESS) {
                    return VK_NULL_HANDLE;
                }
                else
                    return surface;
            }
            else {
                SR_ERROR("Vulkan::Init() : window is not support this architecture!");
                return VK_NULL_HANDLE;
            }
    #else
            SR_UNUSED_VARIABLE(window);
            SRHalt("Unsupported platform!");
            return VK_NULL_HANDLE;
    #endif
        };

        if (auto&& pImpl = window->GetImplementation<BasicWindowImpl>()) {
            m_kernel->SetSize(pImpl->GetSurfaceWidth(), pImpl->GetSurfaceHeight());
        }

        if (!m_kernel->Init(createSurf, window->GetHandle(), m_deviceExtensions, true, swapInterval > 0)) {
            SR_ERROR("Vulkan::Init() : failed to initialize Evo Vulkan kernel!");
            return false;
        }

        SR_INFO("Vulkan::Init() : create vulkan memory manager...");
        m_memory = VulkanTools::MemoryManager::Create(m_kernel);
        if (!m_memory) {
            SR_ERROR("Vulkan::Init() : failed to create vulkan memory manager!");
            return false;
        }

        return true;
    }

//   void Vulkan::SetWindowSize(unsigned int w, unsigned int h) {
//       if (SR_UTILS_NS::Debug::Instance().GetLevel() >= SR_UTILS_NS::Debug::Level::Low) {
//           SR_LOG("Vulkan::SetWindowSize() : width = " + std::to_string(w) + "; height = " + std::to_string(h));
//       }

//       m_basicWindow->Resize(w, h);
//   }

//   void Vulkan::SetWindowPosition(int x, int y) {
//       m_basicWindow->Move(x, y);
//   }

    bool Vulkan::PostInit() {
        SR_GRAPH_LOG("Vulkan::PostInit() : post-initializing vulkan...");

        if (!m_kernel->PostInit()) {
            SR_ERROR("Vulkan::PostInit() : failed to post-initialize Evo Vulkan kernel!");
            return false;
        }

        return true;
    }

    bool Vulkan::CompileShader(const std::map<ShaderStage, SR_UTILS_NS::Path>& stages, int32_t FBO, void **shaderData, const std::vector<uint64_t> &uniformSizes) {
        if (FBO < 0) {
            SRHalt("Vulkan::CompileShader() : vulkan required valid FBO for shaders!");
            return false;
        }

        EvoVulkan::Types::RenderPass renderPass = m_kernel->GetRenderPass();
        if (FBO != 0) {
            if (auto fbo = m_memory->m_FBOs[FBO - 1]; fbo) {
                renderPass = fbo->GetRenderPass();
            } else {
                SR_ERROR("Vulkan::CompileShader() : invalid FBO! SOMETHING WENT WRONG! MEMORY MAY BE CORRUPTED!");
                return false;
            }
        }

        if (!renderPass.IsReady()) {
            SR_ERROR("Vulkan::CompileShader() : internal Evo Vulkan error! Render pass isn't ready!");
            return false;
        }

        const int32_t ID = m_memory->AllocateShaderProgram(renderPass);
        if (ID < 0) {
            SR_ERROR("Vulkan::CompileShader() : failed to allocate shader program ID!");
            return false;
        }
        else {
            /// TODO: memory leak possible
            int *dynamicID = new int();
            *dynamicID = ID;
            *shaderData = reinterpret_cast<void *>(dynamicID);
        }

        std::vector<SourceShader> modules = { };

        for (auto&& [shaderStage, stagePath] : stages) {
            SourceShader module(stagePath.ToString(), shaderStage);
            modules.emplace_back(module);
        }

        if (modules.empty()) {
            SRHalt("No shader modules were found!");
            return false;
        }

        auto&& uniforms = SR_GRAPH_NS::AnalyseShader(modules);
        if (!uniforms) {
            SR_ERROR("Vulkan::CompileShader() : failed to analyse shader!");
            return false;
        }

        std::vector<VkDescriptorSetLayoutBinding> descriptorLayoutBindings = {};
        {
            VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
            VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL;

            for (auto &&uniform : uniforms.value()) {
                switch (uniform.type) {
                    case LayoutBinding::Sampler2D: type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; break;
                    case LayoutBinding::Uniform: type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
                    default:
                        SR_ERROR("Vulkan::CompileShader() : unknown binding type!");
                        return false;
                }

                switch (uniform.stage) {
                    case ShaderStage::Vertex: stage = VK_SHADER_STAGE_VERTEX_BIT; break;
                    case ShaderStage::Fragment: stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
                    case ShaderStage::Geometry: stage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
                    case ShaderStage::Compute: stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
                    default:
                        SR_ERROR("Vulkan::CompileShader() : unknown binding stage!\n\tStage: " + SR_UTILS_NS::EnumReflector::ToString(uniform.stage));
                        return false;
                }

                for (auto &&descriptor : descriptorLayoutBindings)
                {
                    if (descriptor.binding == uniform.binding)
                    {
                        if (descriptor.descriptorType != type)
                        {
                            SRHalt("Vulkan::CompileShader() : descriptor types are different! \n\tBinding: " + SR_UTILS_NS::ToString(uniform.binding));
                            return false;
                        }

                        descriptor.stageFlags |= stage;
                        goto skip;
                    }
                }

                descriptorLayoutBindings.emplace_back(EvoVulkan::Tools::Initializers::DescriptorSetLayoutBinding(
                        type, stage, uniform.binding
                ));

            skip:
                SR_NOOP;
            }
        }

        std::vector<EvoVulkan::Complexes::SourceShader> vkModules;
        for (auto&& module : modules) {
            VkShaderStageFlagBits stage = VulkanTools::VkShaderShaderTypeToStage(module.m_stage);
            vkModules.emplace_back(EvoVulkan::Complexes::SourceShader(module.m_path, stage));
        }

        if (!m_memory->m_ShaderPrograms[ID]->Load(
                SR_UTILS_NS::ResourceManager::Instance().GetResPath().Concat("/Cache/Shaders"),
                vkModules,
                descriptorLayoutBindings,
                uniformSizes
        )) {
            DeleteShader(ID);
            SR_ERROR("Vulkan::CompileShader() : failed to load Evo Vulkan shader!");
            return false;
        }

        return true;
    }

    bool Vulkan::LinkShader(
            SR_SHADER_PROGRAM *shaderProgram,
            void **shaderData,
            const SRShaderCreateInfo& shaderCreateInfo) const {
        if (!shaderData) {
            SR_ERROR("Vulkan::LinkShader() : shader data is nullptr!");
            return false;
        }
        int *dynamicID = reinterpret_cast<int *>(*shaderData);
        if (!dynamicID) {
            SR_ERROR("Vulkan::LinkShader() : dynamic ID is nullptr!");
            return false;
        }

        auto &&vkVertexDescriptions = VulkanTools::AbstractVertexDescriptionsToVk(shaderCreateInfo.vertexDescriptions);
        auto &&vkVertexAttributes = VulkanTools::AbstractAttributesToVkAttributes(shaderCreateInfo.vertexAttributes);
        if (vkVertexAttributes.size() != shaderCreateInfo.vertexAttributes.size()) {
            SR_ERROR("Vulkan::LinkShader() : vkVertexDescriptions size != vertexDescriptions size!");
            delete dynamicID;
            return false;
        }

        if (!m_memory->m_ShaderPrograms[*dynamicID]->SetVertexDescriptions(vkVertexDescriptions, vkVertexAttributes)) {
            SR_ERROR("Vulkan::LinkShader() : failed to set vertex descriptions!");
            delete dynamicID;
            return false;
        }

        /** Так как геометрия грузится отзеркаленная по оси X, то она выворачивается наизнанку,
         * соответственно, нужно изменить отсечения полигонов на обратный */
        ///const CullMode cullMode =
        ///        shaderCreateInfo.cullMode == CullMode::Back ? CullMode::Front :
        ///            (shaderCreateInfo.cullMode == CullMode::Front ? CullMode::Back : shaderCreateInfo.cullMode);

        const CullMode cullMode = shaderCreateInfo.cullMode;

        if (!shaderCreateInfo.Validate()) {
            SR_ERROR("Vulkan::LinkShader() : failed to validate shader create info! Create info:"
                     "\n\tPolygon mode: " + SR_UTILS_NS::EnumReflector::ToString(shaderCreateInfo.polygonMode) +
                     "\n\tCull mode: " + SR_UTILS_NS::EnumReflector::ToString(cullMode) +
                     "\n\tDepth compare: " + SR_UTILS_NS::EnumReflector::ToString(shaderCreateInfo.depthCompare) +
                     "\n\tPrimitive topology: " + SR_UTILS_NS::EnumReflector::ToString(shaderCreateInfo.primitiveTopology)
            );

            return false;
        }

        const VkSampleCountFlagBits sampleCount = m_currentVkFramebuffer ? m_currentVkFramebuffer->GetSampleCount() : m_kernel->GetDevice()->GetMSAASamples();
        const bool depthEnabled = m_currentVkFramebuffer ? m_currentVkFramebuffer->IsDepthEnabled() : true;

        if (!m_memory->m_ShaderPrograms[*dynamicID]->Compile(
                VulkanTools::AbstractPolygonModeToVk(shaderCreateInfo.polygonMode),
                VulkanTools::AbstractCullModeToVk(cullMode),
                VulkanTools::AbstractDepthOpToVk(shaderCreateInfo.depthCompare),
                shaderCreateInfo.blendEnabled && depthEnabled,
                shaderCreateInfo.depthWrite,
                shaderCreateInfo.depthTest,
                VulkanTools::AbstractPrimitiveTopologyToVk(shaderCreateInfo.primitiveTopology),
                sampleCount)
        ) {
            SR_ERROR("Vulkan::LinkShader() : failed to compile Evo Vulkan shader!");
            delete dynamicID;
            return false;
        }

        *shaderProgram = *dynamicID;

        delete dynamicID;
        return true;
    }

    bool Vulkan::CreateFrameBuffer(const SR_MATH_NS::IVector2 &size, int32_t &FBO, DepthLayer *pDepth, std::vector<ColorLayer> &colors, uint8_t sampleCount) {
        std::vector<int32_t> colorBuffers;
        colorBuffers.reserve(colors.size());

        std::vector<VkFormat> formats;
        formats.reserve(colors.size());

        for (auto&& color : colors) {
            colorBuffers.emplace_back(color.texture);
            formats.emplace_back(VulkanTools::AbstractTextureFormatToVkFormat(color.format));
        }

        if (size.x == 0 || size.y == 0) {
            SR_ERROR("Vulkan::CreateFrameBuffer() : width or height equals zero!");
            return false;
        }

        if (FBO == 0) {
            SR_ERROR("Vulkan::CreateFrameBuffer() : zero frame buffer are default frame buffer!");
            return false;
        }

        std::optional<int32_t> depthBuffer = pDepth ? pDepth->texture : std::optional<int32_t>();

        if (FBO > 0) {
            if (!m_memory->ReAllocateFBO(FBO - 1, size.x, size.y, colorBuffers, depthBuffer, sampleCount)) {
                SR_ERROR("Vulkan::CreateFrameBuffer() : failed to re-allocate frame buffer object!");
            }
            goto success;
        }

        FBO = m_memory->AllocateFBO(size.x, size.y, formats, colorBuffers, depthBuffer, sampleCount) + 1;
        if (FBO <= 0) {
            FBO = SR_ID_INVALID;
            SR_ERROR("Vulkan::CreateFrameBuffer() : failed to allocate FBO!");
            return false;
        }

    success:
        if (pDepth && depthBuffer.has_value()) {
            pDepth->texture = depthBuffer.value();
        }
        else if (pDepth) {
            pDepth->texture = SR_ID_INVALID;
        }

        for (uint32_t i = 0; i < static_cast<uint32_t>(colors.size()); ++i) {
            colors[i].texture = colorBuffers[i];
        }

        return true;
    }

    [[nodiscard]] bool Vulkan::FreeTextures(int32_t *IDs, uint32_t count) const {
        if (!IDs) {
            SR_ERROR("Vulkan::FreeTextures() : texture IDs is nullptr!");
            return false;
        }

        for (uint32_t i = 0; i < count; i++) {
            if (IDs[i] < 0) {
                SR_ERROR("Vulkan::FreeTextures() : texture ID less zero!");
                return false;
            }

            if (!m_memory->FreeTexture((uint32_t) IDs[i])) {
                SR_ERROR("Vulkan::FreeTextures() : failed to free texture!");
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] bool Vulkan::FreeFBO(uint32_t FBO) const {
        return this->m_memory->FreeFBO(FBO - 1);
    }

    int32_t Vulkan::CalculateTexture(
            uint8_t *data,
            ColorFormat format,
            uint32_t w,
            uint32_t h,
            TextureFilter filter,
            TextureCompression compression,
            uint8_t mipLevels,
            bool alpha,
            bool cpuUsage// unused
    ) const {
        auto vkFormat = VulkanTools::AbstractTextureFormatToVkFormat(format);
        if (vkFormat == VK_FORMAT_MAX_ENUM) {
            SR_ERROR("Vulkan::CalculateTexture() : unsupported format!");
            return -1;
        }

        if (compression != TextureCompression::None) {
            vkFormat = VulkanTools::AbstractTextureCompToVkFormat(compression, vkFormat);
            if (vkFormat == VK_FORMAT_MAX_ENUM) {
                SR_ERROR("Vulkan::CalculateTexture() : unsupported format with compression!");
                return -1;
            }

            if (auto sz = MakeGoodSizes(w, h); sz != std::pair(w, h)) {
                data = ResizeToLess(w, h, sz.first, sz.second, data);
                w = sz.first;
                h = sz.second;
            }

            if (data == nullptr || w == 0 || h == 0) {
                SR_ERROR("Vulkan::CalculateTexture() : failed to reconstruct image!");
                return -1;
            }

            SR_LOG("Vulkan::CalculateTexture() : compress " + std::to_string(w * h * 4 / 1024 / 1024) +
                   "MB source image...");

            if (data = Graphics::Compress(w, h, data, compression); data == nullptr) {
                SR_ERROR("Vulkan::CalculateTexture() : failed to compress image!");
                return -1;
            }
        }

        auto ID = this->m_memory->AllocateTexture(
                data, w, h, vkFormat,
                VulkanTools::AbstractTextureFilterToVkFilter(filter),
                compression, mipLevels, cpuUsage);

        if (compression != TextureCompression::None)
            free(data); //! free compressed data. Original data isn't will free

        if (ID < 0) {
            SR_ERROR("Vulkan::CalculateTexture() : failed to allocate texture!");
            return -1;
        }

        return ID;
    }

    bool Vulkan::InitGUI() {
        SR_GRAPH_LOG("Vulkan::InitGUI() : initializing ImGUI library...");

        if (!m_kernel->GetDevice()) {
            SR_ERROR("Vulkan::InitGUI() : device is nullptr!");
            return false;
        }

        if (!m_imgui->Init(m_kernel)) {
            SR_ERROR("Vulkan::Init() : failed to init imgui!");
            return false;
        }

        return true;
    }

    bool Vulkan::StopGUI() {
        SR_VULKAN_MSG("Vulkan::StopGUI() : stopping gui...");

        EVSafeFreeObject(m_imgui) else {
            SR_ERROR("Vulkan::StopGUI() : failed to destroy vulkan imgui!");
            return false;
        }

        return true;
    }

    bool Vulkan::BeginDrawGUI() {
        ImGui_ImplVulkan_NewFrame();

    #ifdef SR_WIN32
        ImGui_ImplWin32_NewFrame();
    #endif

        ImGui::NewFrame();

        ImGuizmo::BeginFrame();
        ImGuizmo::Enable(true);

        return true;
    }

    void Vulkan::EndDrawGUI() {
        ImGui::Render();

        ImGuiIO &io = ImGui::GetIO();
        (void) io;

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    InternalTexture Vulkan::GetTexture(uint32_t id) const {
        auto texture = m_memory->m_textures[id];
        if (!texture)
            return {};
        return {
                .m_data   = reinterpret_cast<void *>(texture->GetImageView()),
                .m_width  = texture->GetWidth(),
                .m_height = texture->GetHeight()
        };
    }

    /*int32_t Vulkan::GetImGuiTextureDescriptorFromTexture(uint32_t textureID) const {
        auto descriptorSet = m_memory->AllocateDynamicTextureDescriptorSet(ImGui_ImplVulkan_GetDescriptorSetLayout(), textureID);
        if (descriptorSet < 0) {
            Helper::Debug::Error("Vulkan::GetImGuiTextureDescriptorFromTexture() : failed to allocate dynamic texture descriptor set!");
            return -1;
        }
        else
            return descriptorSet;
    }

    void *Vulkan::GetDescriptorSet(uint32_t id) const { return reinterpret_cast<void*>(m_memory->m_descriptorSets[id].m_self); }

    void *Vulkan::GetDescriptorSetFromDTDSet(uint32_t id) const {
        return reinterpret_cast<void*>(m_memory->GetDynamicTextureDescriptorSet(id));
    }*/

    SR_MATH_NS::FColor Vulkan::GetPixelColor(uint64_t textureId, uint32_t x, uint32_t y) {
        if (textureId == SR_ID_INVALID || textureId >= m_memory->m_countTextures.first) {
            return SR_MATH_NS::FColor(0.f);
        }

        auto&& pTexture = m_memory->m_textures[textureId];
        if (!pTexture) {
            return SR_MATH_NS::FColor(0.f);
        }

        auto&& pixel = pTexture->GetPixel(x, y, 0);
        return SR_MATH_NS::FColor(
                static_cast<SR_MATH_NS::Unit>(pixel.r),
                static_cast<SR_MATH_NS::Unit>(pixel.g),
                static_cast<SR_MATH_NS::Unit>(pixel.b),
                static_cast<SR_MATH_NS::Unit>(pixel.a)
        );
    }

    int32_t Vulkan::CalculateVBO(void *vertices, Vertices::VertexType type, size_t count) {
        const auto size = Vertices::GetVertexSize(type);
        if (auto id = m_memory->AllocateVBO(size * count, vertices); id >= 0) {
            return id;
        } else
            return SR_ID_INVALID;
    }

    int32_t Vulkan::CalculateIBO(void *indices, uint32_t indxSize, size_t count, int32_t VBO) {
        /// ignore VBO
        if (auto id = m_memory->AllocateIBO(indxSize * count, indices); id >= 0) {
            return id;
        }
        else
            return SR_ID_INVALID;
    }

    SR_MATH_NS::IVector2 Vulkan::GetScreenSize() const {
        /// TODO: это нужно?
        //     return m_basicWindow->GetScreenResolution();
        return SR_MATH_NS::IVector2();
    }

    uint64_t Vulkan::GetVRAMUsage() {
        return m_kernel->GetAllocator() ? m_kernel->GetAllocator()->GetGPUMemoryUsage() : 0;
    }

    void Vulkan::SetViewport(int32_t width, int32_t height) {
        if (width > 0 && height > 0) {
            m_viewport = EvoVulkan::Tools::Initializers::Viewport(
                    static_cast<float_t>(width),
                    static_cast<float_t>(height),
                    0.f, 1.f
            );
        }
        else {
            if (m_currentFBOid == 0) {
                m_viewport = m_kernel->GetViewport();
            }
            else if (m_currentFramebuffer) {
                m_viewport = m_currentVkFramebuffer->GetViewport();
            }
        }

        vkCmdSetViewport(m_currentCmd, 0, 1, &m_viewport);
    }

    void Vulkan::SetScissor(int32_t width, int32_t height) {
        if (width > 0 && height > 0) {
            m_scissor = EvoVulkan::Tools::Initializers::Rect2D(width, height, 0, 0);
        }
        else {
            if (m_currentFBOid == 0) {
                m_scissor = m_kernel->GetScissor();
            }
            else if (m_currentFramebuffer) {
                m_scissor = m_currentVkFramebuffer->GetScissor();
            }
        }

        vkCmdSetScissor(m_currentCmd, 0, 1, &m_scissor);
    }

    int32_t Vulkan::AllocateShaderProgram(const SRShaderCreateInfo& createInfo, int32_t fbo) {
        void* temp = nullptr;

        auto&& sizes = std::vector<uint64_t>();
        sizes.reserve(createInfo.uniforms.size());
        for (auto&& [binding, size] : createInfo.uniforms)
            sizes.push_back(size);

        if (!CompileShader(createInfo.stages, fbo, &temp, sizes)) {
            SR_ERROR("Vulkan::AllocateShaderProgram() : failed to compile shader stages!");
            return SR_ID_INVALID;
        }

        int32_t program = SR_ID_INVALID;

        if (!LinkShader(&program, &temp, createInfo)) {
            SR_ERROR("Vulkan::AllocateShaderProgram() : failed linking shader!");
            return false;
        }

        return program;
    }

    uint8_t Vulkan::GetSmoothSamplesCount() const {
        return m_kernel->GetDevice()->GetMSAASamplesCount();
    }

    //!-----------------------------------------------------------------------------------------------------------------

    bool SRVulkan::OnResize()  {
        vkQueueWaitIdle(m_device->GetQueues()->GetGraphicsQueue());
        vkDeviceWaitIdle(*m_device);

        Environment::Get()->SetBuildState(false);

        uint32_t w = m_width;
        uint32_t h = m_height;

        /// TODO: это нужно?
        // Environment::Get()->g_callback(Environment::WinEvents::Resize, Environment::Get()->GetBasicWindow(), &w, &h);

        if (m_GUIEnabled) {
            dynamic_cast<Framework::Graphics::Vulkan *>(Environment::Get())->GetVkImGUI()->ReSize(w, h);
        }

        return true;
    }

    EvoVulkan::Core::RenderResult SRVulkan::Render()  {
        if (PrepareFrame() == EvoVulkan::Core::FrameResult::OutOfDate) {
            VK_LOG("SRVulkan::Render() : out of date...");
            m_hasErrors |= !ResizeWindow();

            if (m_hasErrors) {
                return EvoVulkan::Core::RenderResult::Fatal;
            }

            VK_LOG("SRVulkan::Render() : window are successfully resized!");

            return EvoVulkan::Core::RenderResult::Success;
        }

        for (const auto& submitInfo : m_framebuffersQueue) {
            if (auto result = vkQueueSubmit(m_device->GetQueues()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS) {
                VK_ERROR("renderFunction() : failed to queue submit (frame buffer)! Reason: " + EvoVulkan::Tools::Convert::result_to_description(result));
                return EvoVulkan::Core::RenderResult::Error;
            }
        }

        auto&& vkImgui = dynamic_cast<Framework::Graphics::Vulkan*>(Environment::Get())->GetVkImGUI();

        m_submitCmdBuffs[0] = m_drawCmdBuffs[m_currentBuffer];
        if (m_GUIEnabled && vkImgui && !vkImgui->IsSurfaceDirty()) {
            m_submitCmdBuffs[1] = vkImgui->Render(m_currentBuffer);
            m_submitInfo.commandBufferCount = 2;
        } 
        else {
            m_submitInfo.commandBufferCount = 1;
        }

        m_submitInfo.waitSemaphoreCount = 1;
        if (m_waitSemaphore) {
            m_submitInfo.pWaitSemaphores = &m_waitSemaphore;
        }
        else
            m_submitInfo.pWaitSemaphores = &m_syncs.m_presentComplete;

        m_submitInfo.pCommandBuffers = m_submitCmdBuffs;
        m_submitInfo.pSignalSemaphores = &m_syncs.m_renderComplete;

        /// Submit to queue
        if (auto result = vkQueueSubmit(m_device->GetQueues()->GetGraphicsQueue(), 1, &m_submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS) {
            VK_ERROR("renderFunction() : failed to queue submit! Reason: " + EvoVulkan::Tools::Convert::result_to_description(result));

            if (result == VK_ERROR_DEVICE_LOST) {
                SR_UTILS_NS::Debug::Instance().Terminate();
            }

            return EvoVulkan::Core::RenderResult::Error;
        }

        switch (SubmitFrame()) {
            case EvoVulkan::Core::FrameResult::Success:
                return EvoVulkan::Core::RenderResult::Success;

            case EvoVulkan::Core::FrameResult::Error:
                return EvoVulkan::Core::RenderResult::Error;

            case EvoVulkan::Core::FrameResult::OutOfDate: {
                m_hasErrors |= !ResizeWindow();

                if (m_hasErrors) {
                    return EvoVulkan::Core::RenderResult::Fatal;
                }
                else {
                    return EvoVulkan::Core::RenderResult::Success;
                }
            }
            case EvoVulkan::Core::FrameResult::DeviceLost:
                SR_UTILS_NS::Debug::Instance().Terminate();

            default: {
                SRAssertOnce(false);
                return EvoVulkan::Core::RenderResult::Fatal;
            }
        }
    }

    bool SRVulkan::IsRayTracingRequired() const noexcept {
    #ifdef SR_ANDROID
        return false;
    #else
        return SR_UTILS_NS::Features::Instance().Enabled("RayTracing", false);
    #endif

    }
}