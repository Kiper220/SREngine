//
// Created by Monika on 22.01.2023.
//

#ifndef SRENGINE_SRSL_SHADER_H
#define SRENGINE_SRSL_SHADER_H

#include <Graphics/SRSL/RefAnalyzer.h>
#include <Graphics/SRSL/ICodeGenerator.h>
#include <Graphics/SRSL/ShaderType.h>
#include <Graphics/Types/Vertices.h>
#include <Graphics/Pipeline/IShaderProgram.h>

namespace SR_SRSL_NS {
    SR_ENUM_NS_CLASS_T(ShaderLanguage, uint8_t,
        PseudoCode, GLSL, HLSL, Metal
    );

    struct SRSLSampler {
        std::string type;
        bool isPublic = false;
        uint64_t binding = 0;
    };
    typedef std::map<std::string, SRSLSampler> SRSLSamplers;

    struct SRSLUniformBlock {
        struct Field {
            std::string type;
            std::string name;
            uint64_t size = 0;
            uint64_t alignedSize = 0;
            bool isPublic = false;
        };

        uint64_t size = 0;
        uint64_t binding = 0;

        std::vector<Field> fields;
    };

    /** Это не шейдер в привычном понимании, это набор всех данных для генерирования любого
     * шейдерного кода и для последующей его экспортации. */
    class SRSLShader : public SR_UTILS_NS::NonCopyable {
        using Ptr = std::shared_ptr<SRSLShader>;
        using Super = SR_UTILS_NS::NonCopyable;
        using UniformBlocks = std::map<std::string, SRSLUniformBlock>;
    private:
        explicit SRSLShader(SR_UTILS_NS::Path path, SRSLAnalyzedTree::Ptr&& pAnalyzedTree);

    public:
        SR_NODISCARD static SRSLShader::Ptr Load(SR_UTILS_NS::Path path);

    public:
        SR_NODISCARD std::string ToString(ShaderLanguage shaderLanguage) const;
        SR_NODISCARD bool Export(ShaderLanguage shaderLanguage) const;

        SR_NODISCARD bool IsCacheActual() const;

        SR_NODISCARD const SRSLUniformBlock* FindUniformBlock(const std::string& name) const;
        SR_NODISCARD const SRSLUniformBlock::Field* FindField(const std::string& name) const;
        SR_NODISCARD Vertices::VertexType GetVertexType() const;
        SR_NODISCARD SR_SRSL_NS::ShaderType GetType() const;
        SR_NODISCARD const SRSLAnalyzedTree::Ptr GetAnalyzedTree() const;
        SR_NODISCARD const SRSLUseStack::Ptr GetUseStack() const;
        SR_NODISCARD const UniformBlocks& GetUniformBlocks() const { return m_uniformBlocks; }
        SR_NODISCARD const SRSLSamplers& GetSamplers() const { return m_samplers; }
        SR_NODISCARD const SRShaderCreateInfo& GetCreateInfo() const { return m_createInfo; }
        SR_NODISCARD const std::map<std::string, SRSLVariable*>& GetShared() const { return m_shared; }
        SR_NODISCARD const std::map<std::string, SRSLVariable*>& GetConstants() const { return m_constants; }

    private:
        SR_NODISCARD ISRSLCodeGenerator::SRSLCodeGenRes GenerateStages(ShaderLanguage shaderLanguage) const;

        bool Prepare();
        bool PrepareSettings();
        bool PrepareUniformBlocks();
        bool PrepareSamplers();
        bool PrepareStages();

    private:
        SR_UTILS_NS::Path m_path;

        std::map<std::string, SRSLVariable*> m_shared;
        std::map<std::string, SRSLVariable*> m_constants;
        ShaderType m_type = ShaderType::Unknown;
        SRShaderCreateInfo m_createInfo;
        SRSLAnalyzedTree::Ptr m_analyzedTree;
        SRSLUseStack::Ptr m_useStack;
        UniformBlocks m_uniformBlocks;
        SRSLSamplers m_samplers;

    };
}

#endif //SRENGINE_SRSL_SHADER_H
