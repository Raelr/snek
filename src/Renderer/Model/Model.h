#pragma once

#include "../Device/VulkanDevice.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace SnekVk 
{
    /**
     * @brief The model class contains all rendering data required by our shaders. Models
     * contain all relevant vertex information, alongside a buffer containing our vertices. 
     */
    class Model
    {
        public:

        struct PushConstantData
        {
            glm::mat4 transform {1.0f};
            alignas(16) glm::vec3 color {0.0f, 0.0f, 0.0f};
        };

        /**
         * @brief The Vertex struct represents all the vertex data that'll be passed into our shaders.
         * Right now all we expect is a vector 2 for it's position in 2D space. 
         */
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;

            /**
             * @brief Get a list of binding colorDescriptions for this Vertex. A binding description details
             * the size of the data being sent into the GPU and the type of data being sent in.
             * 
             * @return All vertex description information in the form of a std::array<VkVertexInputBindingDescription, 1> 
             */
            static std::array<VkVertexInputBindingDescription, 1> GetBindingDescriptions();

            /**
             * @brief Gets all attribute colorDescriptions for our vertex. This describes where the
             * data should be sent to and the format in which it is expected to arrive in. 
             * 
             * @return The attribute colorDescriptions in the form of a std::array<VkVertexInputAttributeDescription, 1>
             */
            static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions();
        };

        struct Data 
        {
            Vertex* vertices;
            u32 vertexCount;
            u32* indices;
            u32 indexCount;
        };

        // 'Structors

        /**
         * @brief Constructor for creating a new model.
         * 
         * @param device The VulkanDevice being used for rendering. 
         * @param configData a struct specifying the config information required to create a model
         */
        Model(VulkanDevice& device, const Data& configData);
        Model(VulkanDevice& device);
        ~Model();

        void DestroyModel();

        // Model is not copyable - we just delete the copy constructors.
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        void SetVertices(const Vertex* vertices, u32 vertexCount);

        /**
         * @brief Binds the model's buffers to the current command buffer. 
         * 
         * When using the same model across multiple objects, it is only necessary
         * to bind the vertex buffers once and simply pass in any dynamic data. 
         * This function handles any vertex-specific bindings that need to occur.
         * 
         * @param commandBuffer The command buffer being used for image recording. 
         */
        void Bind(VkCommandBuffer commandBuffer);

        /**
         * @brief Draws the current set vertices (and writes them to the currently bound vertex buffer).
         * 
         * @param commandBuffer The command buffer being used to draw the image
         */
        void Draw(VkCommandBuffer commandBuffer);

        private:

        /**
         * @brief Create a vertx buffer that can store our per-vertex information and upload it to the GPU. 
         * 
         * @param vertices An array of all vertices in model space.
         */
        void CreateVertexBuffers(const Vertex* vertices);
        void CreateIndexBuffer(const u32* indices);
        
        VulkanDevice& device;
        
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        u32 vertexCount;

        bool hasIndexBuffer = false;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        u32 indexCount;
    };
}
