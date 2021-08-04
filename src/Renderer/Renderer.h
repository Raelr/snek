#pragma once

#include "Device/VulkanDevice.h"
#include "Swapchain/Swapchain.h"
#include "Pipeline/Pipeline.h"
#include "Model/Model.h"

#include <vector>

namespace SnekVk 
{
    class Renderer
    {
        public:
            Renderer(SnekVk::Window& window);
            ~Renderer();

            void SubmitModel(Model* model);
            void CreateCommandBuffers();
            void DrawFrame();

            VulkanDevice& GetDevice() { return device; }
            SwapChain& GetSwapChain() { return swapChain; }

            void CreatePipelineLayout();
            void ClearDeviceQueue() { vkDeviceWaitIdle(device.Device()); } 
            Pipeline CreateGraphicsPipeline();
        private:
            
            static Utils::Array<VkCommandBuffer> commandBuffers;

            void RecordCommandBuffer(int imageIndex);
            void RecreateSwapChain();
            void FreeCommandBuffers();
            PipelineConfigInfo CreateDefaultPipelineConfig();

            SnekVk::Window& window;
            
            VulkanDevice device;
            SwapChain swapChain;

            std::vector<Model*> models;

            VkPipelineLayout pipelineLayout{nullptr};
            Pipeline graphicsPipeline{CreateGraphicsPipeline()};
    };
}
