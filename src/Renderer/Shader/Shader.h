#pragma once

#include "../Core.h"
#include "../Pipeline/Pipeline.h"
#include "../Buffer/Buffer.h"
#include "../Utils/Hash.h"
#include <map>

namespace SnekVk
{
    class Shader
    {
        public: 

        static constexpr size_t MAX_VERTEX_ATTRIBUTE_SIZE = 5;
        static constexpr size_t MAX_UNIFORM_SIZE = 5;

        Shader();
        Shader(const char* filePath, PipelineConfig::PipelineStage stage);
        ~Shader();

        struct VertexBinding
        {
            static constexpr size_t MAX_VERTEX_ATTRIBUTES = 10;
            size_t attributeCount = 0;
            u32 vertexStride = 0;
            VertexDescription::Attribute attributes[MAX_VERTEX_ATTRIBUTES];
        };

        template<typename T, size_t S>
        struct Storage
        {
            static constexpr size_t MAX_COUNT = S;
            size_t count = 0; 
            T data[MAX_COUNT];
        };

        struct VertexStorage
        {
            static constexpr size_t MAX_VERTEX_BINDINGS = 5;
            size_t bindingCount = 0; 
            VertexBinding bindings[MAX_VERTEX_BINDINGS];
        };

        template<typename T> 
        struct Uniform
        {
            Utils::StringId id = 0;
            u32 location = 0;
            u32 binding = 0; 
            u64 size;
        };

        void AddVertexAttribute(u32 binding, u32 offset, VertexDescription::AttributeType type);

        void SetVertexInputSize(u32 binding, u32 offset);
        void SetShaderPath(const char* filePath);

        void SetUniformStruct(u32 location, u32 binding, const char* name, u64 size);
        void SetUniformStruct(u32 location, u32 binding, Utils::StringId strId, u64 size);

        u32 GetUniformStructIdx(const char* name);
        u32 GetUniformStructIdx(Utils::StringId strId);

        Storage<Uniform<const void*>, MAX_UNIFORM_SIZE> GetUniformStructs() { return uniformStructs; }
        Storage<VertexBinding, MAX_VERTEX_ATTRIBUTE_SIZE> GetVertexBindings() { return vertexStorage; }
        PipelineConfig::PipelineStage GetStage() { return stage; }
        const char* GetPath() { return filePath; }

        private:

        Storage<Uniform<const void*>, MAX_UNIFORM_SIZE> uniformStructs;
        Storage<VertexBinding, MAX_VERTEX_ATTRIBUTE_SIZE> vertexStorage;
        
        const char* filePath;
        PipelineConfig::PipelineStage stage;
    };
}