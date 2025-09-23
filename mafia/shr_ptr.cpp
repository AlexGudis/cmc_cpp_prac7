#include <iostream>

//  https://codereview.stackexchange.com/questions/254279/c-shared-ptr-implementation



template<class T>
class Shared_pointer {
    T* stored_ptr;                   // Указатель объект
    unsigned long long* counter;     // счётчик ссылок
    
public:
    // из указателя
    Shared_pointer(T* ptr) {
        stored_ptr = ptr;
        counter = new unsigned long long{0};
        if (stored_ptr) {           
            (*counter)++;
        }
    }

    // конструктор копирования
    Shared_pointer(const Shared_pointer& ptr) {
        stored_ptr = ptr.stored_ptr; 
        counter = ptr.counter;        
        (*counter)++;                 
    }
    
    Shared_pointer operator=(const Shared_pointer& ptr) {
        // проверка что это не самоприсваивание (a = a)
        if (stored_ptr != ptr.stored_ptr) {
            reset();
            stored_ptr = ptr.stored_ptr; 
            counter = ptr.counter;        
            (*counter)++;                 
        }
        return *this;                
    }

    // разыменования 
    T& operator*() const {
        return *stored_ptr;          
    }

    //  стрелки 
    T* operator->() const {
        return stored_ptr;           
    }

    
    auto operator==(const Shared_pointer& other) const {
        return stored_ptr == other.stored_ptr;  
    }

    // Сразу для <, >, == C++20 функция
    auto operator<=>(const Shared_pointer& other) const {
        return stored_ptr <=> other.stored_ptr;
    }
    
    T* get() const {
        return stored_ptr;     
    }

    void reset() {
        if (counter == nullptr) return;
        
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
        if (counter == nullptr) return 0;  
        return *counter;            
    }
    
    ~Shared_pointer() {
        if (stored_ptr) {           
            reset();                  
        }
    }
};

// обертка. сразу создаёт объект типа т и оборачивает в умный указатель
/*
Args&&... - универсальная ссылка, которая может принимать как lvalue, так и rvalue.
std::forward сохраняет категорию значения (lvalue/rvalue), переданного в функцию

// Вместо этого:
Shared_pointer<MyClass> ptr1(new MyClass(42, "hello"));

// Можно писать так:
auto ptr1 = make_shared_pointer<MyClass>(42, "hello");

*/
template<typename T, typename... Args>
Shared_pointer<T> make_shared_pointer(Args&&... args) {
    return Shared_pointer<T>(new T{std::forward<Args>(args)...});
}