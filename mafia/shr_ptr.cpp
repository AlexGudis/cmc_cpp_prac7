#include <iostream>
// Based on https://codereview.stackexchange.com/questions/254279/c-shared-ptr-implementation
/*
Реализовать свою версию умного указателя, функциональность которого должна
напоминать shared_ptr. Класс реализации должен быть шаблонным, должны быть
реализованы конструкторы, деструкторы, присваивание, разыменование (в 2 видах),
reset, swap, get, операции сравнения.
*/
template<class T>
class Shared_pointer {
    T* stored_ptr;
    unsigned long long* counter;
public:
    Shared_pointer(T* ptr) {
        stored_ptr = ptr;
        counter = new unsigned long long{0};
        if (stored_ptr) {
            (*counter)++;
        }
    }

    Shared_pointer(const Shared_pointer& ptr) {
        stored_ptr = ptr.stored_ptr;
        counter = ptr.counter;
        (*counter)++;
    }
    
    Shared_pointer operator=(const Shared_pointer& ptr) {
        if (stored_ptr != ptr.stored_ptr) {
            reset();
            stored_ptr = ptr.stored_ptr;
            counter = ptr.counter;
            (*counter)++;
        }
        return *this;
    }

    T& operator*() const {
        return *stored_ptr;
    }

    T* operator->() const {
        return stored_ptr;
    }

    auto operator==(const Shared_pointer& other) const {
        return stored_ptr == other.stored_ptr;
    }

    auto operator<=>(const Shared_pointer& other) const {
        return stored_ptr <=> other.stored_ptr;
    }

    friend std::ostream& operator<<(std::ostream& os, const Shared_pointer& ptr) {
        os << ptr.stored_ptr;
        return os;
    }
    
    T* get() const {
        return stored_ptr;
    }

    void reset() {
        (*counter)--;
        if (*counter == 0) {
            delete counter;
            delete stored_ptr;
        }
        stored_ptr = nullptr;
        counter = nullptr;
    }

    void swap(Shared_pointer& ptr) {
        std::swap(counter, ptr.counter);
        std::swap(stored_ptr, ptr.stored_ptr);
    }

    unsigned long long use_count() const {
        return *counter;
    }
    
    ~Shared_pointer() {
        if (stored_ptr) {
            reset();
        }
    }
};


template<typename T, typename... Args>
Shared_pointer<T> make_shared_pointer(Args&&... args) {
    return Shared_pointer<T>(new T{std::forward<Args>(args)...});
}
