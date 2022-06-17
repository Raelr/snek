#include "Fence.h"
#include "../Device/VulkanDevice.h"

namespace SnekVk
{
    Fence::Fence(const u32& fenceCount)
        : fences{Utils::Array<VkFence>(fenceCount)}
    {
        for(auto& fence: fences) fence = VK_NULL_HANDLE;
    }

    void Fence::DestroyFence() {
        auto device = VulkanDevice::GetDeviceInstance();

        for(auto& fence : fences) vkDestroyFence(device->Device(), fence, nullptr);
    }

    void Fence::WaitForFence(const u32& index, const u64& timeout) {
        auto* device = VulkanDevice::GetDeviceInstance();

        vkWaitForFences(device->Device(), 1, &fences[index], VK_TRUE, timeout);
    }

    void Fence::SetFence(const u32& index, const VkFence& fence) {
        fences[index] = fence;
    }

    const VkFence &Fence::GetFence(const u32 &index) const {
        return fences[index];
    }

    VkFence &Fence::GetFence(const u32 &index) {
        return fences[index];
    }

    void Fence::Initialise() {
        auto* device = VulkanDevice::GetDeviceInstance();

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for(auto& fence : fences)
        {
            SNEK_ASSERT(vkCreateFence(device->Device(), &fenceInfo, nullptr, OUT &fence) == VK_SUCCESS,
                        "Failed to create fences!");
        }
    }
}

