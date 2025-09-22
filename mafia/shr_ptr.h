# pragma once
#include <atomic>


template <typename T>
class MySharedPtr
{
private:
    T* ptr; // Указатель на управляемый объект
    std::atomic<int>* ref_count; // Cчетчик ссылок на разделяемый ресурс
    
    // Вспомогательная функция для освобождения ресурсов
    void release() {
        if (ref_count) {
            // Атомарно уменьшаем счетчик
            --(*ref_count);
            if (*ref_count == 0) {
                delete ptr;          // Удаляем объект
                delete ref_count;    // Удаляем счетчик
            }
        }
    }


    /*
    noexcept - ключевое словое, которое гарантирует, что функция не выбрасывает исключений
    как итог получаем скорость работы быстрее, т.к. компилятор не будет вставлять отладочный код для 
    подготовки к исключению

    explicit - запрет неявного преобразования
    Только через конструктор копирования с приведением к нужному типу
    */

public: 

    // Конструктор по умолчанию
    MySharedPtr() noexcept : ptr(nullptr), ref_count(nullptr) {}

    // Конструктор из сырого указателя
    explicit MySharedPtr(T* p) : ptr(p), ref_count(new std::atomic<int>(1)) {
        if (!p) {
            ref_count = nullptr;
        }
    }

    // Конструктор копирования
    MySharedPtr(const MySharedPtr& other) noexcept : ptr(other.ptr), ref_count(other.ref_count) {
        if (ref_count) {
            ++(*ref_count);  // Увеличиваем счетчик ссылок
        }
    }

    // Конструктор перемещения
    MySharedPtr(MySharedPtr&& other) noexcept : ptr(other.ptr), ref_count(other.ref_count) {
        other.ptr = nullptr;      // Обнуляем источник
        other.ref_count = nullptr;
    }

    // Деструктор
    ~MySharedPtr() {
        release();
    }

    // Оператор присваивания копированием
    MySharedPtr& operator=(const MySharedPtr& other) noexcept {
        if (this != &other) {
            release();              // Освобождаем текущие ресурсы
            ptr = other.ptr;        // Копируем данные
            ref_count = other.ref_count;
            if (ref_count) {
                ++(*ref_count);     // Увеличиваем счетчик
            }
        }
        return *this;
    }

    /*
    Message hello{text1, std::size(text1)};
    hello = Message{text2, std::size(text2)};   // присваивание объекта с перемещением

    Стоит отметить, что, как и в случае с конструктором перемещения, присваиваемое значение представляет rvalue - 
    временный объект в памяти (Message{text2, std::size(text2)};), 
    который после выполнения операции (присовения) будет не нужен. 
    И это как раз идеальный случай для применения оператора присваивания с перемещением
    */
    // Оператор присваивания перемещением
    MySharedPtr& operator=(MySharedPtr&& other) noexcept {
        if (this != &other) {
            release();              // Освобождаем текущие ресурсы
            ptr = other.ptr;        // Перемещаем данные
            ref_count = other.ref_count;
            other.ptr = nullptr;    // Обнуляем источник
            other.ref_count = nullptr;
        }
        return *this;
    }

    // Разыменование
    T& operator*() const noexcept { 
        return *ptr; 
    }
    
    T* operator->() const noexcept { 
        return ptr; 
    }
    
    // Получение сырого указателя (адреса в памяти на объект)
    T* get() const noexcept { 
        return ptr; 
    }

    // Сброс указателя с возможностью передачи нового объекта 
    void reset(T* p = nullptr) noexcept {
        release();                  // Освобождаем текущие ресурсы
        ptr = p;
        ref_count = p ? new std::atomic<int>(1) : nullptr;
    }


    /*
    Под капотом это работает вот так
    namespace std {
    template<typename T>
    void swap(T& a, T& b) {
        T temp = std::move(a);  // Перемещаем a во временную переменную
        a = std::move(b);       // Перемещаем b в a
        b = std::move(temp);    // Перемещаем временную переменную в b
    }
    */
    // Обмен двух указателей
    void swap(MySharedPtr& other) noexcept {
        std::swap(ptr, other.ptr);
        std::swap(ref_count, other.ref_count);
    }


    // const после функции означает, что метод не изменяет объект класса 
    // Получение количества ссылок
    int use_count() const noexcept {
        if (ref_count)
        {
            return *ref_count;
        }
        return 0;
    }
    
    // Проверка на уникальность владения
    bool unique() const noexcept {
        return use_count() == 1;
    }
    
    // Явное преобразование в bool
    explicit operator bool() const noexcept {
        return ptr != nullptr;
    }

    // Операторы сравнения
    bool operator==(const MySharedPtr& other) const noexcept {
        return ptr == other.ptr;
    }
    
    bool operator!=(const MySharedPtr& other) const noexcept {
        return ptr != other.ptr;
    }
    
    bool operator<(const MySharedPtr& other) const noexcept {
        return ptr < other.ptr;
    }
};