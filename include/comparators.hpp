#pragma once

#include "book.hpp"

namespace bookdb::comp {

// ==== Компараторы по возрастанию ====

// Класс прозрачного компаратора по автору книги (лексикографическое сравнение)
struct LessByAuthor {
    using is_transparent = void;  // спец. маркер для поддержки гетерогенного поиска

    // Перегруженные операторы вызова функции
    // а) для сравнения описателей книг по автору
    bool operator()(const Book &lhs, const Book &rhs) const { return lhs.author_ < rhs.author_; }
    // б) для сравнения описателя книги и автора-строки (для гетерогенного поиска)
    bool operator()(const Book &book, std::string_view author) const { return book.author_ < author; }
    // в) для сравнения автора-строки и описателя книги (для гетерогенного поиска)
    bool operator()(std::string_view author, const Book &book) const { return author < book.author_; }
};

// Класс прозрачного компаратора по названию книги (лексикографическое сравнение)
struct LessByTitle {
    using is_transparent = void;  // спец. маркер для поддержки гетерогенного поиска

    // Перегруженные операторы вызова функции
    // а) для сравнения описателей книг по названию
    bool operator()(const Book &lhs, const Book &rhs) const { return lhs.title_ < rhs.title_; }
    // б) для сравнения описателя книги и названия-строки (для гетерогенного поиска)
    bool operator()(const Book &book, std::string_view title) const { return book.title_ < title; }
    // в) для сравнения названия-строки и описателя книги (для гетерогенного поиска)
    bool operator()(std::string_view title, const Book &book) const { return title < book.title_; }
};

// Класс прозрачного компаратора по году издания книги
struct LessByYear {
    using is_transparent = void;  // спец. маркер для поддержки гетерогенного поиска

    // Перегруженные операторы вызова функции
    // а) для сравнения описателей книг по году издания
    bool operator()(const Book &lhs, const Book &rhs) const { return lhs.year_ < rhs.year_; }
    // б) для сравнения описателя книги и года-числа (для гетерогенного поиска)
    bool operator()(const Book &book, int year) const { return book.year_ < year; }
    // в) для сравнения года-числа и описателя книги (для гетерогенного поиска)
    bool operator()(int year, const Book &book) const { return year < book.year_; }
};

// Класс прозрачного компаратора по рейтингу книги
struct LessByRating {
    using is_transparent = void;  // спец. маркер для поддержки гетерогенного поиска

    // Перегруженные операторы вызова функции
    // а) для сравнения описателей книг по рейтингу
    bool operator()(const Book &lhs, const Book &rhs) const { return lhs.rating_ < rhs.rating_; }
    // б) для сравнения описателя книги и рейтинга-числа (для гетерогенного поиска)
    bool operator()(const Book &book, double rating) const { return book.rating_ < rating; }
    // в) для сравнения рейтинга-числа и описателя книги (для гетерогенного поиска)
    bool operator()(double rating, const Book &book) const { return rating < book.rating_; }
};

// Класс прозрачного компаратора по числу прочтений
struct LessByPopularity {
    using is_transparent = void;  // спец. маркер для поддержки гетерогенного поиска

    // Перегруженные операторы вызова функции
    // а) для сравнения описателей книг по числу прочтений
    bool operator()(const Book &lhs, const Book &rhs) const { return lhs.read_count_ < rhs.read_count_; }
    // б) для сравнения описателя книги и числа прочтений (для гетерогенного поиска)
    bool operator()(const Book &book, int read_count) const { return book.read_count_ < read_count; }
    // в) для сравнения числа прочтений и описателя книги (для гетерогенного поиска)
    bool operator()(int read_count, const Book &book) const { return read_count < book.read_count_; }
};

// ==== Компараторы по убыванию ====
// Шаблон-адаптер для сравнения по убыванию (принцип DRY)
template <typename Comp>
struct Greater {
    bool operator()(const auto &lhs, const auto &rhs) const {
        return Comp{}(rhs, lhs);  // меняем порядок
    }
};

// Псевдонимы для greater версий
using GreaterByYear = Greater<LessByYear>;
using GreaterByRating = Greater<LessByRating>;
using GreaterByPopularity = Greater<LessByPopularity>;

}  // namespace bookdb::comp