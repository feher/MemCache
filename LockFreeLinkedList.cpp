//
// Created by gabor on 1.8.2018.
//

#include "LockFreeLinkedList.hpp"

LockFreeLinkedList::~LockFreeLinkedList() {
    LinkedMemBlock * freeBlock;
    while ((freeBlock = popFromFront()) != nullptr) {
        delete freeBlock;
    }
}

std::size_t LockFreeLinkedList::calculateLength() {
    std::size_t len{0};
    for (LinkedMemBlock * item{head.load()}; item != nullptr ; item = item->next, ++len) {
        // Nothing
    }
    return len;
}
