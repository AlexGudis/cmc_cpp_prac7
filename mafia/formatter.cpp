// Based on
// https://stackoverflow.com/questions/70905100/c-concepts-for-numeric-types-and-preventing-instantiation-on-pointers
// https://www.geeksforgeeks.org/how-to-iterate-over-the-elements-of-an-stdtuple-in-c/
#include <iostream>
#include <sstream>
#include <limits>
#include <type_traits>
#include <string>
#include <set>
#include <vector>
#include <utility>
#include <tuple>

template <typename T>
concept IntegerType = requires(T param) {
    requires std::is_integral_v<T>; // Проверка на целочисленный тип
};

struct TPrettyPrinter
{

    TPrettyPrinter() : str{} {}

    std::string Str() const
    {
        return str;
    }

    // числа
    template <IntegerType T>
    TPrettyPrinter &f(const T &value)
    {
        str += std::to_string(value); // Конвертирует число в строку
        return *this;
    }

    // строка
    TPrettyPrinter &f(const std::string &value)
    {
        str += value; // Просто добавляет строку
        return *this;
    }


private:
    std::string str;
};

