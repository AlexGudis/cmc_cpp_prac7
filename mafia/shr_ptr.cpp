#include <atomic>
#include <utility>
#include <iostream>


template <typename T>
class MySharedPtr
{
private:
    T* ptr; // Указатель на управляемый объект
    std::atomic<int>* ref_count;; // Cчетчик ссылок на разделяемый ресурс
    
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



};