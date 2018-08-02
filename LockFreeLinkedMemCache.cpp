//
// Created by gabor on 1.8.2018.
//

#include "LockFreeLinkedMemCache.hpp"

void LockFreeLinkedMemCache::upkeep() {
    // We replace the freeBlocks list with an empty temporary list.
    // The user thread will use this new list to release() blocks to it
    // and to acquire() blocks from it.
    // When upkeep() is finished we discard this list. It's OK to do
    // because it has only free blocks.

    LinkedMemBlock * tempListHead{nullptr};
    LinkedMemBlock * freeBlocksListHead{freeBlocks.exchangeHead(tempListHead)};
    LockFreeLinkedList freeBlocksList{freeBlocksListHead};

    auto freeBlocksCount = freeBlocksList.calculateLength();
    if (freeBlocksCount < minFreeBlocks) {
        grow(freeBlocksList, minFreeBlocks - freeBlocksCount);
    } else if (freeBlocksCount > minFreeBlocks) {
        shrink(freeBlocksList, freeBlocksCount - minFreeBlocks);
    }

    freeBlocks.exchangeHead(freeBlocksList.getHead());

    // Use freeBlocksList's destructor to discard the temporary list.
    freeBlocksList.exchangeHead(tempListHead);
}

void LockFreeLinkedMemCache::grow(LockFreeLinkedList & list, std::size_t blockCount) {
    ++growCount;
    for (std::size_t i{0}; i < blockCount; ++i) {
        list.pushToFront(new LinkedMemBlock{blockSize});
    }
}

void LockFreeLinkedMemCache::shrink(LockFreeLinkedList & list, std::size_t blockCount) {
    for (; blockCount > 0; --blockCount) {
        LinkedMemBlock * freeBlock = list.popFromFront();
        delete freeBlock;
    }
}

std::unique_ptr<LinkedMemBlock> LockFreeLinkedMemCache::acquire() {
    return std::unique_ptr<LinkedMemBlock>(freeBlocks.popFromFront());
}

void LockFreeLinkedMemCache::release(std::unique_ptr<LinkedMemBlock> block) {
    freeBlocks.pushToFront(block.release());
}

