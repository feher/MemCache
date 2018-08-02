//
// Created by feher on 1.8.2018.
//

#include "MemBlock.hpp"

MemBlock::MemBlock(std::size_t blockSize)
    : data{std::unique_ptr<uint8_t[]>(new uint8_t[blockSize])} {
}
