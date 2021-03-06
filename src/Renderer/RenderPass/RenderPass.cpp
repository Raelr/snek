#include "RenderPass.h"

namespace SnekVk
{

    RenderPass::~RenderPass()
    {
        DestroyRenderPass();
    }

    void RenderPass::DestroyRenderPass()
    {
        vkDestroyRenderPass(device->Device(), renderPass, nullptr);
    }

    void
    RenderPass::Begin(VkRenderPass renderPass, VkCommandBuffer commandBuffer, VkFramebuffer frameBuffer, VkOffset2D offset, VkExtent2D extent,
                      VkClearValue *clearValues, u32 clearValueCount)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = frameBuffer;

        renderPassInfo.renderArea.offset = offset;
        renderPassInfo.renderArea.extent = extent;

        renderPassInfo.clearValueCount = clearValueCount;
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(OUT commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPass::End(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderPass(OUT commandBuffer);
    }

    RenderPass::RenderPass() = default;

    void RenderPass::Initialise(VulkanDevice *vulkanDevice, const RenderPass::Config& config)
    {
        device = vulkanDevice;

        auto& attachments = config.GetAttachments();
        auto& subPasses = config.GetSubPasses();
        auto& dependencies = config.GetDependencies();

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachments.Count();
        renderPassCreateInfo.pAttachments = attachments.Data();
        renderPassCreateInfo.subpassCount = subPasses.Count();
        renderPassCreateInfo.pSubpasses = subPasses.Data();
        renderPassCreateInfo.dependencyCount = dependencies.Count();
        renderPassCreateInfo.pDependencies = dependencies.Data();

        SNEK_ASSERT(vkCreateRenderPass(device->Device(), &renderPassCreateInfo, nullptr, OUT &renderPass) == VK_SUCCESS,
                    "Failed to create render pass!")
    }

    void RenderPass::Initialise(VulkanDevice* device, RenderPass &renderpass, const RenderPass::Config &config) {
        renderpass.Initialise(device, config);
    }

    // Config functions.

    RenderPass::Config &RenderPass::Config::WithAttachment(const VkAttachmentDescription& attachment)
    {
        attachments.Append(attachment);
        return *this;
    }

    RenderPass::Config &RenderPass::Config::WithSubPass(const VkSubpassDescription& subpass)
    {
        subpasses.Append(subpass);
        return *this;
    }

    RenderPass::Config &RenderPass::Config::WithDependency(const VkSubpassDependency& dependency)
    {
        dependencies.Append(dependency);
        return *this;
    }
}