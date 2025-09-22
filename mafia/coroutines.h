// coroutines.h
#pragma once
#include <coroutine>
#include <exception>
#include <coroutine>
#include <functional>

// Простая корутина без возвращаемого значения
struct Task {
    struct promise_type {
        Task get_return_object() { return Task{std::coroutine_handle<promise_type>::from_promise(*this)}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> coro;
    
    Task(std::coroutine_handle<promise_type> h) : coro(h) {}
    ~Task() { if (coro) coro.destroy(); }
};

// Корутина с возвращаемым значением
template<typename T>
struct ValueTask {
    struct promise_type {
        T value;
        std::exception_ptr exception;

        ValueTask get_return_object() { 
            return ValueTask{std::coroutine_handle<promise_type>::from_promise(*this)}; 
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_value(T val) { value = val; }
        void unhandled_exception() { exception = std::current_exception(); }
    };

    std::coroutine_handle<promise_type> coro;
    
    ValueTask(std::coroutine_handle<promise_type> h) : coro(h) {}
    ~ValueTask() { if (coro) coro.destroy(); }
    
    T get() {
        if (coro.promise().exception) {
            std::rethrow_exception(coro.promise().exception);
        }
        return coro.promise().value;
    }
};


struct ChoiceAwaiter {
    std::function<int()> choice_provider;
    int result = -1;
    
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> handle) noexcept {
        // Просто получаем выбор сразу
        result = choice_provider();
        handle.resume(); // Немедленно возобновляем
    }
    int await_resume() const noexcept { return result; }
};