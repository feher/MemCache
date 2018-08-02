//
// Created by gabor on 1.8.2018.
//

//
// This is a locked implementation of the memory cache.
// It uses a vector of pointers to memory blocks.
// The vector is guarded by a mutex.
//

#ifndef MEM_CACHE_HPP
#define MEM_CACHE_HPP

#include <cstddef>
#include <mutex>
#include <vector>
#include <memory>
#include "MemBlock.hpp"

class MemCache {
public:
    MemCache(std::size_t minFreeBlocks, std::size_t blockSize)
            : minFreeBlocks{minFreeBlocks},
              blockSize{blockSize} {
    }

    MemCache(const MemCache &) = delete;

    virtual ~MemCache() = default;

    void upkeep();

    std::unique_ptr<MemBlock> acquire();

    void release(std::unique_ptr<MemBlock> block);

    std::size_t getFreeBlocksCount();
    std::size_t getAllocationCount();

private:
    std::size_t minFreeBlocks;
    std::size_t blockSize;

    std::size_t growCount{0};
    std::vector<std::unique_ptr<MemBlock>> freeBlocks{};

    std::mutex lock{};

    void grow(std::size_t);
    void shrink(std::size_t);
};

inline std::size_t MemCache::getFreeBlocksCount() {
    return freeBlocks.size();
}

inline std::size_t MemCache::getAllocationCount() {
    return growCount;
}

#endif
