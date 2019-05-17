#include <string>
#include <iostream>

static inline void printStressTestResult(const std::string &message,
                                         std::size_t acquireCount,
                                         std::size_t failedAcquireCount,
                                         std::size_t allocCount)
{
    std::cout << message << std::endl
              << "\tacquire count = " << acquireCount << std::endl
              << "\tfailed acquires = " << failedAcquireCount
              << " (" << (failedAcquireCount * 100.0f / acquireCount) << " %)" << std::endl
              << "\tallocations = " << allocCount
              << " (" << (allocCount * 100.0f / acquireCount) << " %)" << std::endl
              << std::endl;
}
