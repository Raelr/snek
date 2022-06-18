#ifndef SNEK_FENCE_H
#define SNEK_FENCE_H

#include "../Core.h"

namespace SnekVk
{
    /**
     * @brief The fence class is a SOA class for storing per-frame fences. This class is intended to store multiple
     * vulkan fences which can be awaited by passing in an appropriate index. This structure can be seen as a managed array
     * which enables you to access fences within certain positions and then wait on them.
     *
     * By default, no memory is allocated from Vulkan. The reason for this is that sometimes we just want a storage
     * which doesn't create a new fence, but rather just stores an existing one.
     *
     * This class requires memorhy to be manually deallocated via the DestroyFence method.
     */
    class Fence {
    public:
        /**
         * @brief A constructor for creating the Fence collection and allocating the number of fences needed. This
         * constructor will allocate the memory needed to store a number of fences equal to fenceCount.
         * @param fenceCount the number of fences to be stored in this collection.
         */
        explicit Fence(const u32& fenceCount);

        /**
         * @brief Default constructor for the Fence class. This constructor allocates no memory and will create an
         * empty array.
         */
        Fence() = default;

        /**
         * @brief Default destructor for the Fence class. This destructor does nothing. All deallocation logic occurs
         * in the DestroyFence function.
         */
        ~Fence() = default;

        /**
         * @brief Allocates the fences from Vulkan. This will allocate the fences for every element in the array.
         */
        void AllocateFences();

        /**
         * @brief Deallocates the fences.
         */
        void DestroyFence();

        /**
         * @brief Wait for a fence in the collection to become signaled.
         * @param index the index of the fence we want to wait for.
         * @param timeout an amount of time to wait before we stop waiting. By default thios is set to the maximum value
         * of a 64 bit unsigned integer.
         */
        void WaitForFence(const u32& index, const u64& timeout = std::numeric_limits<u64>::max());

        /**
         * @brief Sets the value of the fence at the specified index. This should be used if you want to store an allocated
         * fence into the collection.
         * @param index the position you want to store the fence in.
         * @param fence the VkFence you want to store in the container.
         */
        void SetFence(const u32& index, const VkFence& fence);

        /**
         * @brief Gets the fence stored in the location specified by the index variable.
         * @param index the index of the fence you want to get.
         * @return the fence stored at the specified index.
         */
        const VkFence& GetFence(const u32& index) const;

        /**
         * @brief Gets the fence stored in the location specified by the index variable.
         * @param index the index of the fence you want to get.
         * @return the fence stored at the specified index.
         */
        VkFence& GetFence(const u32& index);

    private:
        Utils::Array<VkFence> fences;
    };

}

#endif //SNEK_FENCE_H
