//
// Created by feher on 1.8.2018.
//

#ifndef LINKED_MEM_BLOCK_HPP
#define LINKED_MEM_BLOCK_HPP

#include <memory>
#include <cstdint>

struct LinkedMemBlock {
    LinkedMemBlock * next{nullptr};

    std::unique_ptr<uint8_t[]> data{nullptr};

    LinkedMemBlock(const LinkedMemBlock &) = delete;

    explicit LinkedMemBlock(std::size_t blockSize);

    virtual ~LinkedMemBlock() = default;

    template<typename T>
    T * getAs();
};

template<typename T>
T * LinkedMemBlock::getAs() {
    return reinterpret_cast<T *>(data.get());
}

#endif
