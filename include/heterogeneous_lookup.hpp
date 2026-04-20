#pragma once

#include <string_view>

namespace bookdb {

// Концепт для любого типа T, который можно преобразовать в std::string_view
template <typename T>
concept StringViewConvertible = std::is_convertible_v<T, std::string_view>;

// Прозрачный компаратор "меньше" для строк
// (для сравнения std::string и std::string_view без создания временных объектов)
struct TransparentStringLess {
    using is_transparent = void;  // спец. маркер для поддержки гетерогенного поиска

    // Перегруженный оператор() с шаблонными параметрами, конвертируемыми в std::string_view
    // (вместо отдельных перегрузок для типов std::string, const char* и std::string_view)
    template <StringViewConvertible T, StringViewConvertible U>
    bool operator()(T &&lhs, U &&rhs) const {
        return std::string_view(lhs) < std::string_view(rhs);
    }
};

// Прозрачный компаратор "равно" для строк
struct TransparentStringEqual {
    using is_transparent = void;

    // Перегруженный оператор() с шаблонными параметрами, конвертируемыми в std::string_view
    // (вместо отдельных перегрузок для типов std::string, const char* и std::string_view)
    template <StringViewConvertible T, StringViewConvertible U>
    bool operator()(T &&lhs, U &&rhs) const {
        return std::string_view(lhs) == std::string_view(rhs);
    }
};

// Прозрачная хэш-функция для строк
struct TransparentStringHash {
    using is_transparent = void;

    // Перегруженный оператор() с шаблонным параметром, конвертируемым в std::string_view
    // (вместо отдельных перегрузок для типов std::string, const char* и std::string_view)
    template <StringViewConvertible T>
    std::size_t operator()(T &&str) const {
        return std::hash<std::string_view>{}(std::string_view(str));
    }
};

}  // namespace bookdb
