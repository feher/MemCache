//
// Created by gabor on 1.8.2018.
//

#ifndef LOCK_FREE_LINKED_MEM_CACHE_HPP
#define LOCK_FREE_LINKED_MEM_CACHE_HPP

#include <cstddef>
#include "LockFreeLinkedList.hpp"

// This is a lock free implementation of the memory cache.
// It uses a singly linked list to manage the free blocks.

class LockFreeLinkedMemCache {
public:
    LockFreeLinkedMemCache(std::size_t minFreeBlocks, std::size_t blockSize)
            : minFreeBlocks{minFreeBlocks},
              blockSize{blockSize} {
    }

    LockFreeLinkedMemCache(const LockFreeLinkedMemCache &) = delete;

    virtual ~LockFreeLinkedMemCache() = default;

    void upkeep();

    std::unique_ptr<LinkedMemBlock> acquire();

    void release(std::unique_ptr<LinkedMemBlock> block);

    std::size_t getFreeBlocksCount();
    std::size_t getAllocationCount();

private:
    std::size_t minFreeBlocks;
    std::size_t blockSize;

    std::size_t growCount{0};
    LockFreeLinkedList freeBlocks{};

    void grow(LockFreeLinkedList & list, std::size_t);
    void shrink(LockFreeLinkedList & list, std::size_t);
};

inline std::size_t LockFreeLinkedMemCache::getFreeBlocksCount() {
    return freeBlocks.calculateLength();
}

inline std::size_t LockFreeLinkedMemCache::getAllocationCount() {
    return growCount;
}

#endif
