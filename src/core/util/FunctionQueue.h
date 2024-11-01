#pragma once
#include <deque>
#include <vector>
#include <functional>

namespace spr {

typedef struct FunctionQueue {
    std::deque<std::function<void()>> functors;

    void push_function(std::function<void()>&& function) {
        functors.push_back(function);
    }

    void execute() {
        for (auto it = functors.begin(); it != functors.end(); it++) {
            (*it)(); 
        }

        functors.clear();
    }
} FunctionQueue;

}