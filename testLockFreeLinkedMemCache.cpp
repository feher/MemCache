//
// Created by feher on 1.8.2018.
//

#include <thread>
#include <iostream>
#include <random>

#include "catch.hpp"

#include "LockFreeLinkedMemCache.hpp"
#include "PlainOldData.hpp"

TEST_CASE("LockFreeLinkedMemCache: Upkeep allocates minFreeBlocks", "[LockFreeLinkedMemCache]") {
    LockFreeLinkedMemCache memCache{100, sizeof(PlainOldData)};
    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 100);
}

TEST_CASE("LockFreeLinkedMemCache: Acquire from empty cache", "[LockFreeLinkedMemCache]") {
    LockFreeLinkedMemCache memCache{100, sizeof(PlainOldData)};
    REQUIRE(memCache.acquire() == nullptr);
}

TEST_CASE("LockFreeLinkedMemCache: Release nullptr", "[LockFreeLinkedMemCache]") {
    LockFreeLinkedMemCache memCache{100, sizeof(PlainOldData)};
    memCache.release(nullptr);
    REQUIRE(memCache.getFreeBlocksCount() == 0);
}

TEST_CASE("LockFreeLinkedMemCache: Acquire decrements free block count", "[LockFreeLinkedMemCache]") {
    LockFreeLinkedMemCache memCache{3, sizeof(PlainOldData)};

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);

    std::unique_ptr<LinkedMemBlock> block{memCache.acquire()};
    REQUIRE(memCache.getFreeBlocksCount() == 2);
}

TEST_CASE("LockFreeLinkedMemCache: Release increments free block count", "[LockFreeLinkedMemCache]") {
    LockFreeLinkedMemCache memCache{3, sizeof(PlainOldData)};

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);

    std::unique_ptr<LinkedMemBlock> block{new LinkedMemBlock{sizeof(PlainOldData)}};
    memCache.release(std::move(block));
    REQUIRE(memCache.getFreeBlocksCount() == 4);
}

TEST_CASE("LockFreeLinkedMemCache: Upkeep allocates at least minFreeBlocks blocks", "[LockFreeLinkedMemCache]") {
    LockFreeLinkedMemCache memCache{3, sizeof(float)};

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);

    std::unique_ptr<LinkedMemBlock> block{memCache.acquire()};
    REQUIRE(memCache.getFreeBlocksCount() == 2);

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);
}

TEST_CASE("LockFreeLinkedMemCache: Upkeep releases unnecessary blocks", "[LockFreeLinkedMemCache]") {
    LockFreeLinkedMemCache memCache{3, sizeof(float)};

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);

    std::unique_ptr<LinkedMemBlock> block{memCache.acquire()};
    REQUIRE(memCache.getFreeBlocksCount() == 2);

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);

    memCache.release(std::move(block));
    REQUIRE(memCache.getFreeBlocksCount() == 4);

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == 3);
}
