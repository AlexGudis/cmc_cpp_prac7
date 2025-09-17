#pragma once
#include <coroutine>
#include <functional>


// Designed with help of this artucle: https://habr.com/ru/articles/798935/
struct Task {
    struct promise_type {
        Task get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};


struct ChoiceAwaiter {
    std::function<int()> get_choice;
    int result = -1;
    
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<>) {}
    int await_resume() { return get_choice ? get_choice() : -1; }
};

//  Choise awaiter?