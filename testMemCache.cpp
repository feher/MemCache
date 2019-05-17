//
// Created by feher on 1.8.2018.
//

#include <thread>
#include <iostream>
#include <random>
#include <atomic>

#include "catch.hpp"

#include "MemCache.hpp"
#include "PlainOldData.hpp"

TEST_CASE("MemCache: Upkeep allocates minFreeBlocks", "[MemCache]") {
    MemCache memCache{100, sizeof(PlainOldData)};
    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 100);
}

TEST_CASE("MemCache: Acquire from empty cache", "[MemCache]") {
    MemCache memCache{100, sizeof(PlainOldData)};
    REQUIRE(memCache.acquire() == nullptr);
}

TEST_CASE("MemCache: Release nullptr", "[MemCache]") {
    MemCache memCache{100, sizeof(PlainOldData)};
    memCache.release(nullptr);
    REQUIRE(memCache.getFreeBlocksCount() == 0);
}

TEST_CASE("MemCache: Acquire decrements free block count", "[MemCache]") {
    MemCache memCache{3, sizeof(PlainOldData)};

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);

    std::unique_ptr<MemBlock> block{memCache.acquire()};
    REQUIRE(memCache.getFreeBlocksCount() == 2);
}

TEST_CASE("MemCache: Release increments free block count", "[MemCache]") {
    MemCache memCache{3, sizeof(PlainOldData)};

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);

    std::unique_ptr<MemBlock> block{new MemBlock{sizeof(PlainOldData)}};
    memCache.release(std::move(block));
    REQUIRE(memCache.getFreeBlocksCount() == 4);
}

TEST_CASE("MemCache: Upkeep allocates at least minFreeBlocks blocks", "[MemCache]") {
    MemCache memCache{3, sizeof(float)};

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);

    std::unique_ptr<MemBlock> block{memCache.acquire()};
    REQUIRE(memCache.getFreeBlocksCount() == 2);

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);
}

TEST_CASE("MemCache: Upkeep releases unnecessary blocks", "[MemCache]") {
    MemCache memCache{3, sizeof(float)};

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);

    std::unique_ptr<MemBlock> block = memCache.acquire();
    REQUIRE(memCache.getFreeBlocksCount() == 2);

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);

    memCache.release(std::move(block));
    REQUIRE(memCache.getFreeBlocksCount() == 4);

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);
}
