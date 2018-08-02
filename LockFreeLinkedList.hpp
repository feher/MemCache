//
// Created by gabor on 1.8.2018.
//

#ifndef LOCKFREE_LINKEDLIST_HPP
#define LOCKFREE_LINKEDLIST_HPP

#include <cstdint>
#include <atomic>
#include "LinkedMemBlock.hpp"

class LockFreeLinkedList {
private:
    std::atomic<LinkedMemBlock *> head{nullptr};

public:
    LockFreeLinkedList() = default;

    explicit LockFreeLinkedList(LinkedMemBlock * head);

    virtual ~LockFreeLinkedList();

    LinkedMemBlock * exchangeHead(LinkedMemBlock * newHead);

    LinkedMemBlock * getHead();

    std::size_t calculateLength();

    void pushToFront(LinkedMemBlock * item);

    LinkedMemBlock * popFromFront();
};

inline LockFreeLinkedList::LockFreeLinkedList(LinkedMemBlock * head) : head{head} {

}

inline LinkedMemBlock * LockFreeLinkedList::exchangeHead(LinkedMemBlock * newHead) {
    return head.exchange(newHead);
}

inline LinkedMemBlock * LockFreeLinkedList::getHead() {
    return head.load();
}

inline void LockFreeLinkedList::pushToFront(LinkedMemBlock * item) {
    if (item == nullptr) {
        return;
    }
    item->next = head.load();

    // Note: If pushToFront() and popFromFront() are always called from
    // the same thread then head.store(item) would be enough.

    // This does: head = item
    while (!head.compare_exchange_weak(item->next, item)) {
        // Nothing
    }
}

inline LinkedMemBlock * LockFreeLinkedList::popFromFront() {
    LinkedMemBlock * item{head.load()};

    // Note: If pushToFront() and popFromFront() are always called from
    // the same thread then we can head.store(item->next) would be enough.

    // This does: head = item->next
    while (item && !head.compare_exchange_weak(item, item->next)) {
        // Nothing
    }

    if (item == nullptr) {
        return nullptr;
    }
    item->next = nullptr;
    return item;
}

#endif
