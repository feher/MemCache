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

TEST_CASE("MemCache: Parallel stress test. Ordered acquire/release", "[MemCache]") {
    const std::size_t minFreeBlocksCount{3};
    MemCache memCache{minFreeBlocksCount, sizeof(float)};

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
            std::unique_ptr<MemBlock> block{memCache.acquire()};
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

    std::cout << "MemCache 1: "
              << "acquire count = " << acquireCount
              << ", failed acquire count = " << failedAcquireCount
              << ", allocation count = " << memCache.getAllocationCount()
              << std::endl;

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == minFreeBlocksCount);
}

TEST_CASE("MemCache: Parallel stress test. Random acquire/release", "[MemCache]") {
    const std::size_t minFreeBlocksCount{3};
    MemCache memCache{minFreeBlocksCount, sizeof(float)};

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
        std::vector<std::unique_ptr<MemBlock>> acquiredBlocks{};
        acquiredBlocks.reserve(maxAcquiredBlockCount);

        while (!isTestOver.load()) {
            const int randomNumber{randomNumberGenerator(mt)};
            if (randomNumber < (maxRandomNumber / 2)) {
                if (acquiredBlocks.size() < maxAcquiredBlockCount) {
                    ++acquireCount;
                    std::unique_ptr<MemBlock> block{memCache.acquire()};
                    if (block != nullptr) {
                        acquiredBlocks.push_back(std::move(block));
                    } else {
                        ++failedAcquireCount;
                    }
                }
            } else {
                if (!acquiredBlocks.empty()) {
                    std::unique_ptr<MemBlock> block{std::move(acquiredBlocks.back())};
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

    std::cout << "MemCache 2: "
              << "acquire count = " << acquireCount
              << ", failed acquire count = " << failedAcquireCount
              << ", allocation count = " << memCache.getAllocationCount()
              << std::endl;

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == minFreeBlocksCount);
}

TEST_CASE(
        "MemCache: Parallel stress test. Random acquire/release. Verify data",
        "[MemCache]") {
    const std::size_t minFreeBlocksCount{100};
    MemCache memCache{minFreeBlocksCount, sizeof(PlainOldData)};

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

        std::vector<std::unique_ptr<MemBlock>> acquiredBlocks{};
        acquiredBlocks.reserve(maxAcquiredBlockCount);

        std::vector<int> verificationDatas{};
        verificationDatas.reserve(maxAcquiredBlockCount);

        while (!isTestOver.load()) {
            const int randomNumber{randomNumberGenerator(mt)};
            if (randomNumber < (maxRandomNumber / 2)) {
                if (acquiredBlocks.size() < maxAcquiredBlockCount) {
                    ++acquireCount;
                    std::unique_ptr<MemBlock> block{memCache.acquire()};
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
                    std::unique_ptr<MemBlock> block{std::move(acquiredBlocks.back())};
                    acquiredBlocks.pop_back();
                    verificationDatas.pop_back();
                    memCache.release(std::move(block));
                }
            }
            for (std::size_t i{0}; i < acquiredBlocks.size(); ++i) {
                const std::unique_ptr<MemBlock> & block{acquiredBlocks[i]};
                int verificationData{verificationDatas[i]};
                REQUIRE(block->getAs<PlainOldData>()->verify(verificationData));
            }
        }
    } };

    std::this_thread::sleep_for(std::chrono::seconds(30));

    isTestOver.store(true);
    upkeeperThread.join();
    userThread.join();

    std::cout << "MemCache 3: "
              << "acquire count = " << acquireCount
              << ", failed acquire count = " << failedAcquireCount
              << ", allocation count = " << memCache.getAllocationCount()
              << std::endl;

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == minFreeBlocksCount);
}
