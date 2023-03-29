#pragma once
#include <deque>
#include <vector>
#include <functional>

namespace spr {

typedef struct FunctionStack {
    std::deque<std::function<void()>> functors;

    void push_function(std::function<void()>&& function) {
        functors.push_back(function);
    }

    void execute() {
        for (auto it = functors.rbegin(); it != functors.rend(); it++) {
            (*it)(); 
        }

        functors.clear();
    }
} FunctionStack;

}