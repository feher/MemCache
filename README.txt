Memory cache experiment
-------------------------
Implements a mechanism for allocating, acquiring and freeing blocks of memory (of any plain-old-data type).
The mechanism supports the following operations:

* Init: An instance of the cache can be initializable with
** A minimum number of memory blocks to be acquirable after running an upkeep function (see below)
** A block size for each of the allocated memory blocks. This size is not a compile-time constant.

* Upkeep: Ensures that there are at least the minimum number of memory blocks available
  after it has finished. It is assumed to be run periodically from one thread, which is a different
  thread from the acquiring thread. This thread may perform memory allocations and blocking
  operations.

* Acquire: Acquires one block of memory. It is assumed to to be run from a
  single thread different from the thread running the upkeep function. The acquire
  function does not allocate memory.

* Release: Returns the acquired memory blocks to the cache. It does not deallocate memory
  directly. That is done by the upkeep function.
  The release function may happen in the same thread as the acquire.

Building and running
----------------------
The supplied CMake file just generates two executables with unit tests
and a few stress tests. The stress tests are not unit tests. They are meant to
stress test the implementation during high loads.

Requirements
* cmake 3.10 or above
* gcc with C++14 support (std::make_unique)

Building
1. mkdir build
2. cd build
3. cmake .. # For debug build: cmake -DCMAKE_BUILD_TYPE=Debug ..
4. make

This will generate
* memCacheTest: Unit tests
* memCacheStressTest: Stress tests

Alternatively you can just copy all the cpp/hpp files into you favorite IDE
as a new project and build it from there.

For unit testing I used Catch2 (https://github.com/catchorg/Catch2).
It's a single-header unit testing framework and the code is in catch.hpp.

Implementation variations
---------------------------
* MemCache: Locked with vector of pointers
* LockFreeLinkedMemCache: Lock free with linked list of pointers

Main classes and files
------------------------
* main: The main executable. It just runs unit tests (i.e. uses the memory caches).

* PlainOldData: Used for testing and verification.

* MemCache: Locked implementation of the cache.
* MemBlock: Used by MemCache. Memory block representation.
* testMemCache: Unit tests for MemCache.

* LockFreeLinkedMemCache: Lock free implementation of the cache.
* LockFreeLinkedList: Used by LockFreeLinkedMemCache. Linked list.
* LinkedMemBlock: Used by LockFreeLinkedMemCache. Memory block representation.
* testLockFreeLinkedMemCache: Unit tests for LockFreeLinkedMemCache.

Discussion
------------
The main reason why all of the implementations use list of pointers
instead of a contiguous memory area for storing the blocks is realloc().
When getting rid of unnecessary free blocks we must release memory. If the
blocks are stored in a contigous memory area then we must use
realloc() to shrink this area. But realloc may not preserve the memory
locations (may copy the contents to a new location). This is bad because
it would invalidate all the acquired memory block pointers (that we handed
out to the user thread already).

The acquire() and release() functions use unique_ptr to express ownership.
I.e. the memory block is owned by the current holder of the pointer.

I use smart pointers when possible to avoid unnecessary manual
destruction (i.e. use RAII when possible).
LockFreeLinkedMemCache internally uses naked pointers because
std::atomic<std::unique_ptr> is not in the C++ standard yet.
MemCache uses std::unique_ptr internally.

LockFreeLinkedMemCache operates on the assumption that free blocks
can be discarded as long as after upkeep() has finished there are
at least minFreeBlocks free blocks available. This makes it possible
to isolate the user thread and the upkeep thread to work on different
free block lists. See the comment in LockFreeLinkedMemCache::upkeep()
for details.

LockFreeLinkedMemCache seems to be free of the ABA problem
(https://en.wikipedia.org/wiki/ABA_problem) due to the
assumptions made. We assume that all uses of acquire()/release() happens
on the user thread. We also make sure that upkeep() and acquire()/release()
never work on the same data.

The upkeep() function aggressively reclaims unnecessary free blocks. This means
that after its completion the number of allocated free blocks is always minFreeBlocks.
So that is the predictable upper bound.

