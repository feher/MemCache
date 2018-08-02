cmake_minimum_required(VERSION 3.12)
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
