//
// Created by feher on 1.8.2018.
//

#include "MemCache.hpp"

void MemCache::upkeep() {
    std::lock_guard<std::mutex> guard{lock};

    auto freeBlocksCount = freeBlocks.size();
    if (freeBlocksCount < minFreeBlocks) {
        grow(minFreeBlocks - freeBlocksCount);
    } else if (freeBlocksCount > minFreeBlocks) {
        shrink(freeBlocksCount - minFreeBlocks);
    }
}

void MemCache::grow(std::size_t blockCount) {
    ++growCount;
    for (std::size_t i{0}; i < blockCount; ++i) {
        freeBlocks.push_back(std::make_unique<MemBlock>(blockSize));
    }
}

void MemCache::shrink(std::size_t blockCount) {
    for (; blockCount > 0; --blockCount) {
        std::unique_ptr<MemBlock> freeBlock{std::move(freeBlocks.back())};
        freeBlocks.pop_back();
    }
}

std::unique_ptr<MemBlock> MemCache::acquire() {
    std::lock_guard<std::mutex> guard{lock};
    if (freeBlocks.empty()) {
        return nullptr;
    }
    std::unique_ptr<MemBlock> block{std::move(freeBlocks.back())};
    freeBlocks.pop_back();
    return block;
}

void MemCache::release(std::unique_ptr<MemBlock> block) {
    if (block == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> guard{lock};
    freeBlocks.push_back(std::move(block));
}

