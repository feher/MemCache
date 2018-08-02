//
// Created by feher on 1.8.2018.
//

#include "LinkedMemBlock.hpp"

LinkedMemBlock::LinkedMemBlock(std::size_t blockSize)
    : data{std::unique_ptr<uint8_t[]>(new uint8_t[blockSize])} {
}

