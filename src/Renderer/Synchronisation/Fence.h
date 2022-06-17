#ifndef SNEK_FENCE_H
#define SNEK_FENCE_H

#include "../Core.h"

namespace SnekVk
{
    class Fence {
    public:
        explicit Fence(const u32& fenceCount);

        Fence() = default;
        ~Fence() = default;

        void Initialise();
        void DestroyFence();
        void WaitForFence(const u32& index, const u64& timeout = std::numeric_limits<u64>::max());
        void SetFence(const u32& index, const VkFence& fence);
        const VkFence& GetFence(const u32& index) const;
        VkFence& GetFence(const u32& index);

    private:
        Utils::Array<VkFence> fences;
    };

}

#endif //SNEK_FENCE_H
