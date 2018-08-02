//
// Created by feher on 1.8.2018.
//

#ifndef MEM_BLOCK_HPP
#define MEM_BLOCK_HPP

#include <memory>
#include <cstdint>

struct MemBlock {
    std::unique_ptr<uint8_t[]> data{nullptr};

    MemBlock(const MemBlock &) = delete;

    explicit MemBlock(std::size_t blockSize);

    virtual ~MemBlock() = default;

    template<typename T>
    T * getAs();
};

template<typename T>
T * MemBlock::getAs() {
    return reinterpret_cast<T*>(data.get());
}

#endif
