//
// Created by Monika on 10.06.2022.
//

#include <Memory/UBOManager.h>
#include <Types/Camera.h>
#include <Environment/Environment.h>

namespace SR_GRAPH_NS::Memory {
    UBOManager::UBOManager() {
        m_virtualTable.max_load_factor(0.9f);
        m_virtualTable.reserve(5000);
        m_singleCameraMode = SR_UTILS_NS::Features::Instance().Enabled("SingleCameraMode", false);
    }

    void UBOManager::SetCurrentCamera(Types::Camera *pCamera) {
        m_camera = m_singleCameraMode ? nullptr : pCamera;
    }

    UBOManager::VirtualUBO UBOManager::AllocateUBO(uint32_t uboSize, uint32_t samples) {
        if (!m_camera && !m_singleCameraMode && !m_ignoreCameras) {
            SR_ERROR("UBOManager::AllocateUBO() : camera is nullptr!");
            return SR_ID_INVALID;
        }

        auto&& env = Environment::Get();
        auto&& shaderProgram = env->GetCurrentShaderId();

        if (shaderProgram == SR_ID_INVALID) {
            SR_ERROR("UBOManager::AllocateUBO() : shader program do not set!");
            return SR_ID_INVALID;
        }

        const VirtualUBO virtualUbo = GenerateUnique();

        Descriptor descriptor = SR_ID_INVALID;
        UBO ubo = SR_ID_INVALID;

        if (!AllocMemory(&ubo, &descriptor, uboSize, samples, shaderProgram)) {
            SR_ERROR("UBOManager::AllocateUBO() : failed to allocate memory!");
            return SR_ID_INVALID;
        }

        VirtualUBOInfo virtualUboInfo;

        virtualUboInfo.m_samples = samples;
        virtualUboInfo.m_uboSize = uboSize;
        virtualUboInfo.m_shaderProgram = shaderProgram;

        virtualUboInfo.m_data.emplace_front(VirtualUBOInfo::Data {
                m_ignoreCameras ? nullptr : m_camera,
                descriptor,
                ubo
        });

        m_virtualTable.insert(std::make_pair(
                virtualUbo,
                std::move(virtualUboInfo))
        );

        return virtualUbo;
    }

    bool UBOManager::FreeUBO(UBOManager::VirtualUBO *virtualUbo) {
        if (!SRVerifyFalse(!virtualUbo)) {
            return false;
        }

        auto&& pIt = m_virtualTable.find(*virtualUbo);
        if (pIt == std::end(m_virtualTable)) {
            SRHalt("UBOManager::FreeUBO() : ubo not found!");
            return false;
        }

        auto&& env = Environment::Get();

        auto&& info = pIt->second;
        for (auto&& [pCamera, descriptor, ubo] : info.m_data) {
            FreeMemory(&ubo, &descriptor);
        }

        m_virtualTable.erase(pIt);

        *virtualUbo = SR_ID_INVALID;

        return true;
    }

    UBOManager::VirtualUBO UBOManager::GenerateUnique() const {
        volatile VirtualUBO virtualUbo = SR_ID_INVALID;

        auto&& random = SR_UTILS_NS::Random::Instance();

        while (true) {
            VirtualUBO unique = random.Int32();

            /// можно использовать только положительные индексы
            if (unique < 0) {
                unique = -unique;
            }

            SRAssertOnce(unique >= 0);

            if (m_virtualTable.count(unique) == 0 && unique != SR_ID_INVALID) {
                virtualUbo = unique;
                break;
            }

            SR_WARN("UBOManager::GenerateUnique() : collision detected!");
        }

        return virtualUbo;
    }

    bool UBOManager::AllocMemory(UBO *ubo, Descriptor *descriptor, uint32_t uboSize, uint32_t samples, int32_t shader) {
        auto&& env = Environment::Get();
        auto&& shaderIdStash = env->GetCurrentShaderId();

        env->SetCurrentShaderId(shader);

        if (uboSize > 0) {
            if (*descriptor = env->AllocDescriptorSet({DescriptorType::Uniform}); *descriptor < 0) {
                SR_ERROR("UBOManager::AllocMemory() : failed to allocate descriptor set!");
                goto fails;
            }

            if (*ubo = env->AllocateUBO(uboSize); *ubo < 0) {
                SR_ERROR("UBOManager::AllocMemory() : failed to allocate uniform buffer object!");
                goto fails;
            }
        }
        else if (samples > 0) {
            if (*descriptor = env->AllocDescriptorSet({DescriptorType::CombinedImage}); *descriptor < 0) {
                SR_ERROR("UBOManager::AllocMemory() : failed to allocate descriptor set!");
                goto fails;
            }
        }

        SRAssert(*ubo != SR_ID_INVALID || *descriptor != SR_ID_INVALID);

        env->SetCurrentShaderId(shaderIdStash);
        return true;

    fails:
        env->SetCurrentShaderId(shaderIdStash);
        return false;
    }

    UBOManager::BindResult UBOManager::BindUBO(VirtualUBO virtualUbo) noexcept {
        if (!m_camera && !m_singleCameraMode && !m_ignoreCameras) {
            SR_ERROR("UBOManager::AllocateUBO() : camera is nullptr!");
            return BindResult::Failed;
        }

        auto&& pIt = m_virtualTable.find(virtualUbo);
        if (pIt == std::end(m_virtualTable)) {
            SRHalt("UBOManager::BindUBO() : ubo not found!");
            return BindResult::Failed;
        }

        BindResult result = BindResult::Success;

        auto&& info = pIt->second;

        Descriptor descriptor = SR_ID_INVALID;
        UBO ubo = SR_ID_INVALID;

        for (auto&& data : info.m_data) {
            if (data.pCamera == (m_ignoreCameras ? nullptr : m_camera)) {
                descriptor = data.descriptor;
                ubo = data.ubo;
                break;
            }
        }

        /// если не нашли камеру, то дублируем память под новую камеру
        if (descriptor == SR_ID_INVALID && ubo == SR_ID_INVALID)
        {
            if (!AllocMemory(&ubo, &descriptor, info.m_uboSize, info.m_samples, info.m_shaderProgram)) {
                SR_ERROR("UBOManager::BindUBO() : failed to allocate memory!");
                return BindResult::Failed;
            }

            info.m_data.emplace_front(VirtualUBOInfo::Data {
                    m_ignoreCameras ? nullptr : m_camera,
                    descriptor,
                    ubo
            });

            result = BindResult::Duplicated;
        }

        auto&& env = Environment::Get();

        if (ubo != SR_ID_INVALID) {
            env->BindUBO(ubo);
        }

        if (descriptor != SR_ID_INVALID) {
            env->BindDescriptorSet(descriptor);
        }

        return result;
    }

    UBOManager::VirtualUBO UBOManager::ReAllocateUBO(VirtualUBO virtualUbo, uint32_t uboSize, uint32_t samples) {
        if (virtualUbo == SR_ID_INVALID) {
            return AllocateUBO(uboSize, samples);
        }

        auto&& env = Environment::Get();
        auto&& shaderProgram = env->GetCurrentShaderId();

        if (shaderProgram == SR_ID_INVALID) {
            SR_ERROR("UBOManager::ReAllocateUBO() : shader program do not set!");
            return virtualUbo;
        }

        auto&& pIt = m_virtualTable.find(virtualUbo);
        if (pIt == std::end(m_virtualTable)) {
            SRHalt("UBOManager::ReAllocateUBO() : ubo not found!");
            return virtualUbo;
        }

        auto&& info = pIt->second;

        /// задаем новые значения для дублирования памяти
        info.m_uboSize = uboSize;
        info.m_samples = samples;
        info.m_shaderProgram = shaderProgram;

        /// очищаем ВСЕ старые данные
        for (auto&& [pCamera, descriptor, ubo] : info.m_data) {
            FreeMemory(&ubo, &descriptor);
        }
        info.m_data.clear();

        /// выделяем ОДНУ новую юниформу с дескриптором
        Descriptor descriptor = SR_ID_INVALID;
        UBO ubo = SR_ID_INVALID;

        if (!AllocMemory(&ubo, &descriptor, info.m_uboSize, info.m_samples, info.m_shaderProgram)) {
            SR_ERROR("UBOManager::ReAllocateUBO() : failed to allocate memory!");
        }

        info.m_data.emplace_front(VirtualUBOInfo::Data {
                m_ignoreCameras ? nullptr : m_camera,
                descriptor,
                ubo
        });

        return virtualUbo;
    }

    void UBOManager::FreeMemory(UBOManager::UBO *ubo, UBOManager::Descriptor *descriptor) {
        auto&& env = Environment::Get();

        if (*ubo != SR_ID_INVALID && !env->FreeUBO(ubo)) {
            SR_ERROR("UBOManager::FreeMemory() : failed to free ubo!");
        }

        if (*descriptor != SR_ID_INVALID && !env->FreeDescriptorSet(descriptor)) {
            SR_ERROR("UBOManager::FreeMemory() : failed to free descriptor!");
        }
    }

    void UBOManager::SetIgnoreCameras(bool value) {
        SR_LOCK_GUARD
        m_ignoreCameras = value;
    }
}