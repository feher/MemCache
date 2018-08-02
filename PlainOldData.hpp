//
// Created by feher on 1.8.2018.
//

#ifndef PLAIN_OLD_DATA_HPP
#define PLAIN_OLD_DATA_HPP

#include <string>

struct PlainOldData {

    int i{};
    float f{};
    double d{};

    void set(int v) {
        i = v;
        f = v * 0.5f; // This is just a magic constant.
        d = v * 1.3; // This is just a magic constant.
    }

    bool verify(int v) {
        return (i == v)
                && (f == v * 0.5f)
                && (d == v * 1.3);
    }

};

#endif
