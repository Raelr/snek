#pragma once

#include "../../Core.h"
#include "../../Material/Material.h"
#include "../../Model/Model.h"

namespace SnekVk
{
    class BillboardRenderer
    {
        public:

        BillboardRenderer();
        ~BillboardRenderer();

        void Initialise(const char* globalDataAttributeName, const u64& globalDataSize);
        void Destroy();

        void DrawBillboard(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& colour);

        void Render(VkCommandBuffer& commandBuffer, const u64& globalDataSize, const void* globalData);

        void Flush();

        void RecreateMaterials();

        private:

        struct BillboardVertex 
        {
            glm::vec3 position;
            glm::vec4 colour;
        };

        struct BillboardUBO
        {
            alignas(16) glm::vec3 position;
            alignas(16) glm::vec3 scale;
        };

        u32 billboardCount;

        Material billboardMaterial;
        Model billboardModel;

        Utils::StringId globalDataId;
        Utils::StringId positionsId;

        Utils::StackArray<BillboardVertex, Mesh::MAX_VERTICES> vertices;
        Utils::StackArray<u32, Mesh::MAX_INDICES> indices;
        Utils::StackArray<BillboardUBO, 1000> positions;
    };
}