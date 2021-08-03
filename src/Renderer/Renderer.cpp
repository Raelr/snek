#include "Renderer.h"

namespace SnekVk
{
    VkCommandBuffer* Renderer::commandBuffers;

    Renderer::Renderer(SnekVk::Window& window) 
        : window{window}, 
        device{VulkanDevice(window)}, 
        swapChain{SwapChain(device,  
        window.GetExtent())}
    {
        models.reserve(100);
        commandBuffers = new VkCommandBuffer[swapChain.GetImageCount()];
        CreateCommandBuffers(graphicsPipeline);
    }
    
    Renderer::~Renderer() 
    {
        vkDestroyPipelineLayout(device.Device(), pipelineLayout, nullptr);
    }

    void Renderer::SubmitModel(Model* model)
    {
        models.emplace_back(model);
    }

    void Renderer::CreateCommandBuffers(Pipeline& pipeline)
    {
        u32 size = swapChain.GetImageCount();

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = device.GetCommandPool();
        allocInfo.commandBufferCount = size;

        SNEK_ASSERT(vkAllocateCommandBuffers(device.Device(), &allocInfo, OUT commandBuffers) == VK_SUCCESS,
            "Failed to allocate command buffer");
    }

    void Renderer::RecordCommandBuffer(int imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        SNEK_ASSERT(vkBeginCommandBuffer(OUT commandBuffers[imageIndex], &beginInfo) == VK_SUCCESS,
            "Failed to begin recording command buffer");
        
        u32 clearValueCount = 2;
            VkClearValue clearValues[clearValueCount];
            clearValues[0].color = clearValue;
            clearValues[1].depthStencil = {1.0f, 0};

        RenderPass::Begin(swapChain.GetRenderPass()->GetRenderPass(),
                          OUT commandBuffers[imageIndex],
                          swapChain.GetFrameBuffer(imageIndex),
                          {0,0},
                          swapChain.GetSwapChainExtent(),
                          clearValues,
                          clearValueCount);

        graphicsPipeline.Bind(commandBuffers[imageIndex]);

        for (auto& model : models)
        {
            model->Bind(commandBuffers[imageIndex]);
            model->Draw(commandBuffers[imageIndex]);
        }
        
        RenderPass::End(commandBuffers[imageIndex]);

        SNEK_ASSERT(vkEndCommandBuffer(OUT commandBuffers[imageIndex]) == VK_SUCCESS,
            "Failed to record command buffer!");
    }

    void Renderer::RecreateSwapChain()
    {
        auto extent = window.GetExtent();
        while(extent.width == 0 || extent.height == 0)
        {
            extent = window.GetExtent();
            window.WaitEvents();
        }

        // Clear our graphics pipeline before swapchain re-creation
        graphicsPipeline.ClearPipeline();

        // Re-create swapchain
        swapChain.RecreateSwapchain();

        // Re-create the pipeline once the swapchain renderpass 
        // becomes available again.
        graphicsPipeline.RecreatePipeline(
            "shaders/simpleShader.vert.spv",
            "shaders/simpleShader.frag.spv",
            CreateDefaultPipelineConfig()
        );
    }

    PipelineConfigInfo Renderer::CreateDefaultPipelineConfig()
    {
        auto pipelineConfig = Pipeline::DefaultPipelineConfig(swapChain.GetWidth(), swapChain.GetHeight());
        pipelineConfig.renderPass = swapChain.GetRenderPass()->GetRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;

        return pipelineConfig;
    }

    void Renderer::CreatePipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; 
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        SNEK_ASSERT(vkCreatePipelineLayout(device.Device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS, 
            "Failed to create pipeline layout!");
    }

    Pipeline Renderer::CreateGraphicsPipeline()
    {
        CreatePipelineLayout();

        return SnekVk::Pipeline(
            device, 
            "shaders/simpleShader.vert.spv", 
            "shaders/simpleShader.frag.spv", 
            CreateDefaultPipelineConfig()
        );
    }

    void Renderer::DrawFrame()
    {
        u32 imageIndex;
        auto result = swapChain.AcquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            RecreateSwapChain();
            return;
        }

        SNEK_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, 
            "Failed to acquire swapchain image!");
        
        RecordCommandBuffer(imageIndex);
        result = swapChain.SubmitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            window.WasResized())
        {
            window.ResetWindowResized();
            RecreateSwapChain();
            return;
        }

        SNEK_ASSERT(result == VK_SUCCESS, "Failed to submit command buffer for drawing!");
    }
}
