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

TEST_CASE("LockFreeLinkedMemCache: Parallel stress test. Ordered acquire/release", "[LockFreeLinkedMemCache]") {
    const std::size_t minFreeBlocksCount{100};
    LockFreeLinkedMemCache memCache{minFreeBlocksCount, sizeof(float)};

    std::atomic<bool> isTestOver{false};
    std::size_t acquireCount{0};
    std::size_t failedAcquireCount{0};

    std::thread upkeeperThread{ [&] {
        while (!isTestOver.load()) {
            memCache.upkeep();
        }
    } };

    std::thread userThread{ [&] {
        while (!isTestOver.load()) {
            ++acquireCount;
            std::unique_ptr<LinkedMemBlock> block{memCache.acquire()};
            if (block != nullptr) {
                memCache.release(std::move(block));
            } else {
                ++failedAcquireCount;
            }
        }
    } };

    std::this_thread::sleep_for(std::chrono::seconds(5));

    isTestOver.store(true);
    upkeeperThread.join();
    userThread.join();

    std::cout << "LockFreeLinkedMemCache 1: "
              << "acquire count = " << acquireCount
              << ", failed acquire count = " << failedAcquireCount
              << ", allocation count = " << memCache.getAllocationCount()
              << std::endl;

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == minFreeBlocksCount);
}

TEST_CASE("LockFreeLinkedMemCache: Parallel stress test. Random acquire/release", "[LockFreeLinkedMemCache]") {
    const std::size_t minFreeBlocksCount{100};
    LockFreeLinkedMemCache memCache{minFreeBlocksCount, sizeof(float)};

    std::atomic<bool> isTestOver{false};
    std::size_t acquireCount{0};
    std::size_t failedAcquireCount{0};

    std::thread upkeeperThread{ [&] {
        while (!isTestOver.load()) {
            memCache.upkeep();

            // Simulate some work/delay
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } };

    std::thread userThread{ [&] {
        const int maxRandomNumber{1000};
        std::mt19937 mt{1729};
        std::uniform_int_distribution<int> randomNumberGenerator{1, maxRandomNumber};

        const int maxAcquiredBlockCount{100000};
        std::vector<std::unique_ptr<LinkedMemBlock>> acquiredBlocks{};
        acquiredBlocks.reserve(maxAcquiredBlockCount);

        while (!isTestOver.load()) {
            const int randomNumber{randomNumberGenerator(mt)};
            const bool shouldAcquire{randomNumber < (maxRandomNumber / 2)};
            if (shouldAcquire) {
                if (acquiredBlocks.size() < maxAcquiredBlockCount) {
                    ++acquireCount;
                    std::unique_ptr<LinkedMemBlock> block{memCache.acquire()};
                    if (block != nullptr) {
                        acquiredBlocks.push_back(std::move(block));
                    } else {
                        ++failedAcquireCount;
                    }
                }
            } else {
                if (!acquiredBlocks.empty()) {
                    std::unique_ptr<LinkedMemBlock> block{std::move(acquiredBlocks.back())};
                    acquiredBlocks.pop_back();
                    memCache.release(std::move(block));
                }
            }
        }
    } };

    std::this_thread::sleep_for(std::chrono::seconds(30));

    isTestOver.store(true);
    upkeeperThread.join();
    userThread.join();

    std::cout << "LockFreeLinkedMemCache 2: "
              << "acquire count = " << acquireCount
              << ", failed acquire count = " << failedAcquireCount
              << ", allocation count = " << memCache.getAllocationCount()
              << std::endl;

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == minFreeBlocksCount);
}

TEST_CASE(
        "LockFreeLinkedMemCache: Parallel stress test. Random acquire/release. Verify data",
        "[LockFreeLinkedMemCache]") {
    const std::size_t minFreeBlocksCount{100};
    LockFreeLinkedMemCache memCache{minFreeBlocksCount, sizeof(PlainOldData)};

    std::atomic<bool> isTestOver{false};
    std::size_t acquireCount{0};
    std::size_t failedAcquireCount{0};

    std::thread upkeeperThread{ [&] {
        while (!isTestOver.load()) {
            memCache.upkeep();

            // Simulate some work/delay
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } };

    std::thread userThread{ [&] {
        const int maxRandomNumber{1000};
        std::mt19937 mt{1729};
        std::uniform_int_distribution<int> randomNumberGenerator{1, maxRandomNumber};

        const int maxAcquiredBlockCount{100000};

        std::vector<std::unique_ptr<LinkedMemBlock>> acquiredBlocks{};
        acquiredBlocks.reserve(maxAcquiredBlockCount);

        std::vector<int> verificationDatas{};
        verificationDatas.reserve(maxAcquiredBlockCount);

        while (!isTestOver.load()) {
            const int randomNumber{randomNumberGenerator(mt)};
            const bool shouldAcquire{randomNumber < (maxRandomNumber / 2)};
            if (shouldAcquire) {
                if (acquiredBlocks.size() < maxAcquiredBlockCount) {
                    ++acquireCount;
                    std::unique_ptr<LinkedMemBlock> block{memCache.acquire()};
                    if (block != nullptr) {
                        block->getAs<PlainOldData>()->set(randomNumber);
                        acquiredBlocks.push_back(std::move(block));
                        verificationDatas.push_back(randomNumber);
                    } else {
                        ++failedAcquireCount;
                    }
                }
            } else {
                if (!acquiredBlocks.empty()) {
                    std::unique_ptr<LinkedMemBlock> block{std::move(acquiredBlocks.back())};
                    acquiredBlocks.pop_back();
                    verificationDatas.pop_back();
                    memCache.release(std::move(block));
                }
            }
            for (std::size_t i{0}; i < acquiredBlocks.size(); ++i) {
                const std::unique_ptr<LinkedMemBlock> & block = acquiredBlocks[i];
                int verificationData = verificationDatas[i];
                REQUIRE(block->getAs<PlainOldData>()->verify(verificationData));
            }
        }
    } };

    std::this_thread::sleep_for(std::chrono::seconds(30));

    isTestOver.store(true);
    upkeeperThread.join();
    userThread.join();

    std::cout << "LockFreeLinkedMemCache 3: "
              << "acquire count = " << acquireCount
              << ", failed acquire count = " << failedAcquireCount
              << ", allocation count = " << memCache.getAllocationCount()
              << std::endl;

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == minFreeBlocksCount);
}
