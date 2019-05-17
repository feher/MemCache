cmake_minimum_required(VERSION 3.10)
project(memCacheTest)

set(CMAKE_CXX_STANDARD 14)

add_executable(memCacheTest
        main.cpp
        PlainOldData.hpp
        MemBlock.hpp
        MemBlock.cpp
        MemCache.hpp
        MemCache.cpp
        testMemCache.cpp
        LockFreeLinkedList.hpp
        LockFreeLinkedList.cpp
        LockFreeLinkedMemCache.hpp
        LockFreeLinkedMemCache.cpp
        LinkedMemBlock.hpp
        LinkedMemBlock.cpp
        testLockFreeLinkedMemCache.cpp
        )
target_link_libraries(memCacheTest pthread)

add_executable(memCacheStressTest
        main.cpp
        PlainOldData.hpp
        MemBlock.hpp
        MemBlock.cpp
        MemCache.hpp
        MemCache.cpp
        StressTestHelper.hpp
        stressTestMemCache.cpp
        LockFreeLinkedList.hpp
        LockFreeLinkedList.cpp
        LockFreeLinkedMemCache.hpp
        LockFreeLinkedMemCache.cpp
        LinkedMemBlock.hpp
        LinkedMemBlock.cpp
        stressTestLockFreeLinkedMemCache.cpp
        )
target_link_libraries(memCacheStressTest pthread)
