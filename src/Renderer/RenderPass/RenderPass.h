#pragma once

#include "../Core.h"
#include "../Device/VulkanDevice.h"
#include "Utils/Attachments.h"

namespace SnekVk
{
    /**
     * @brief The RenderPass class is a utilitly class which encapsulates the Vulkan RenderPass capabilities. The
     * class holds an instance of a VkRenderPass while handling Vulkan renderpass initialisation, destruction, and
     * recording stages.
     *
     * In Vulkan, a RenderPass represents a single set of rendering instructions. It may be useful to think of renderpasses
     * as different stages in the rendering process where one pass may render objects, another may render shadows, and a
     * third for compute shaders. Renderpasses allow us to segment our operations into logical chunks.
     */
    class RenderPass
    {
    public:
        class Config
        {
        public:
            Config() = default;
            ~Config() = default;

            static constexpr u32 MAX_ATTACHMENTS = 2;
            static constexpr u32 MAX_SUBPASSES = 1;
            static constexpr u32 MAX_DEPENDENCIES = 1;

            Config& WithAttachment(const VkAttachmentDescription& attachment);
            Config& WithSubPass(const VkSubpassDescription& subpass);
            Config& WithDependency(const VkSubpassDependency& dependency);

            const Utils::StackArray<VkAttachmentDescription, MAX_ATTACHMENTS>& GetAttachments() const { return attachments; }
            const Utils::StackArray<VkSubpassDescription, MAX_SUBPASSES>& GetSubPasses() const { return subpasses; }
            const Utils::StackArray<VkSubpassDependency, MAX_DEPENDENCIES>& GetDependencies() const { return dependencies; }
        private:
            Utils::StackArray<VkAttachmentDescription, MAX_ATTACHMENTS> attachments;
            Utils::StackArray<VkSubpassDescription, MAX_SUBPASSES> subpasses;
            Utils::StackArray<VkSubpassDependency, MAX_DEPENDENCIES> dependencies;
        };
        /**
         * @brief An empty constructor. The RenderPass class relies on the Build() method to initialize the Vulkan
         * RenderPass object.
         */
        RenderPass();

        /**
         * @brief Initialises the RenderPass. Creating a RenderPass requires us to explicitly state what operations
         * the RenderPass will be responsible for, along with any graphics stages the renderpass may rely on.
         *
         * @param vulkanDevice The device instance being used to crete the render pass
         * @param config Config variables used to create the renderpass
         */
        void Initialise(VulkanDevice* vulkanDevice, const Config& config);

        static Config CreateConfig() { return Config{}; }

        /**
         * @brief Begins a RenderPass. Any rendering operations that occur will utilise the attachments and operations
         * we specified when initialising the RenderPass. Vulkan will continue to use this RenderPass for rendering images
         * until we instruct it to stop by using the End() function.
         *
         * @param renderPass The renderpass we wish to render with
         * @param commandBuffer The command buffer used to record the commands
         * @param frameBuffer The framebuffer used to store our rendering data
         * @param offset Specifies any offsets to be used to the positions of rendered objects
         * @param extent The size of the rendered area.
         * @param clearValues A list of clear values. When a frame is cleared, Vulkan will fill the space with the colors specified here
         * @param clearValueCount The number of clear values provided
         */
        static void Begin(VkRenderPass renderPass,
                          VkCommandBuffer commandBuffer,
                          VkFramebuffer frameBuffer,
                          VkOffset2D offset,
                          VkExtent2D extent,
                          VkClearValue* clearValues,
                          u32 clearValueCount);

        /**
         * @brief Ends the RenderPass. Calling this will consolidate all the rendering data into the RenderPass and allow
         * a new RenderPass to be recorded.
         *
         * @param commandBuffer The command buffer currently being recorded to
         */
        static void End(VkCommandBuffer commandBuffer);

        /**
         * @brief Returns the Vulkan RenderPass object.
         *
         * @return the VkRenderPass object held by this instance
         */
        VkRenderPass GetRenderPass() { return renderPass; }

        /**
         * @brief Cleans up the Vulkan RenderPass object.
         */
        ~RenderPass();

        /**
         * @brief Function for explicitly destroying the Vulkan RenderPass object. Should only be used when the destruction
         * sequence of objects needs to be manually handled.
         */
        void DestroyRenderPass();

    private:
        VkRenderPass renderPass {VK_NULL_HANDLE};
        VulkanDevice* device {nullptr};
    };
}

