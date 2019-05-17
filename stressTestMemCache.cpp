#include <thread>
#include <iostream>
#include <random>
#include <atomic>

#include "catch.hpp"

#include "MemCache.hpp"
#include "PlainOldData.hpp"
#include "StressTestHelper.hpp"

#define TEST_NAME "MemCache: Parallel stress test. Ordered acquire/release"
TEST_CASE(TEST_NAME, "[MemCache]") {
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

    printStressTestResult(TEST_NAME, acquireCount, failedAcquireCount, memCache.getAllocationCount());

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == minFreeBlocksCount);
}
#undef TEST_NAME

#define TEST_NAME "MemCache: Parallel stress test. Random acquire/release"
TEST_CASE(TEST_NAME, "[MemCache]") {
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

    printStressTestResult(TEST_NAME, acquireCount, failedAcquireCount, memCache.getAllocationCount());

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == minFreeBlocksCount);
}
#undef TEST_NAME

#define TEST_NAME "MemCache: Parallel stress test. Random acquire/release. Verify data"
TEST_CASE(TEST_NAME, "[MemCache]") {
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

    printStressTestResult(TEST_NAME, acquireCount, failedAcquireCount, memCache.getAllocationCount());

    memCache.upkeep();
    REQUIRE(memCache.getFreeBlocksCount() == minFreeBlocksCount);
}
#undef TEST_NAME
