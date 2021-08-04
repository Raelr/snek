#pragma once

#include "../Device/VulkanDevice.h"

namespace SnekVk
{
    class SwapChain
    {
        public:

        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        // 'Structors 

        SwapChain(VulkanDevice& device, VkExtent2D windowExtent);
        ~SwapChain();

        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain*) = delete;

        // Getters

        VkFormat GetSwapChainImageFormat() { return swapChainImageFormat; }
        VkExtent2D GetSwapChainExtent() { return swapChainExtent; }

        VkFramebuffer GetFrameBuffer(u32 i) { return swapChainFrameBuffers[i]; }

        u32 GetImageCount() { return imageCount; }
        VkSwapchainKHR GetSwapChain() { return swapChain; }

        u32 GetWidth() { return swapChainExtent.width; }
        u32 GetHeight() { return swapChainExtent.height; }

        VkRenderPass GetRenderPass() { return renderPass; }

        VkResult AcquireNextImage(u32* imageIndex);
        VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, u32* imageIndex);

        void ClearSwapChain(bool isRecreated = false);
        void RecreateSwapchain();

        private:
        
        void CreateSwapChain();
        void CreateImageViews();
        void CreateRenderPass();
        void CreateDepthResources();
        void CreateFrameBuffers();
        void CreateSyncObjects();

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(VkSurfaceFormatKHR* formats, size_t formatCount);
        VkPresentModeKHR ChoosePresentMode(VkPresentModeKHR* presentModes, size_t presentModeCount);
        VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities);
        VkFormat FindDepthFormat();
        void Init();
        
        VulkanDevice& device;
        VkExtent2D windowExtent;

        VkFramebuffer* swapChainFrameBuffers {nullptr};
        VkRenderPass renderPass {nullptr};

        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        // Depth image data.
        // NOTE: If these values do end up being changed dynamically, it might be a good idea to
        // wrap them in a vector.
        VkImage* depthImages {nullptr};
        VkDeviceMemory* depthImageMemorys {nullptr};
        VkImageView* depthImageViews {nullptr};

        VkImage* swapChainImages {nullptr};
        VkImageView* swapChainImageViews {nullptr};
        u32 imageCount;

        VkSwapchainKHR swapChain {nullptr};

        VkSemaphore* imageAvailableSemaphores {nullptr};
        VkSemaphore* renderFinishedSemaphores {nullptr};
        VkFence* inFlightFences {nullptr};
        VkFence* imagesInFlight {nullptr};
        size_t currentFrame = 0;
    };
}