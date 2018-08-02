Building and running
----------------------
Due to time constraints I used the 30 day evaluation
version of CLion (https://www.jetbrains.com/clion/).
For simplicity, I advise you to just download CLion and open this project in it.

Alternatively you can just copy all the cpp/hpp files into you favorite IDE
as a new project and build it from there.

For unit testing I used Catch2 (https://github.com/catchorg/Catch2).
It's a single-header unit testing framework and the code is in catch.hpp.

Implementation variations
---------------------------
* MemCache: Locked with vector of pointers
* LockFreeLinkedMemCache: Lock free with linked list of pointers

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

