#pragma once

#include <algorithm>
#include <functional>

#include "book.hpp"      // заголовочный файл с классом описателя книги и шаблонами для его форматирования
#include "concepts.hpp"  // концепты для работы с описателями книг

namespace bookdb {

// === Фабрики унарных предикатов ===

// Предикат совпадения жанра книги
inline auto GenreIs(Genre genre) {
    return [genre](const Book &book) { return book.GetGenre() == genre; };
}

// Предикат превышения рейтинга книги заданного порога
inline auto RatingAbove(double threshold) {
    return [threshold](const Book &book) { return book.GetRating() >= threshold; };
}

// Предикат нахождения года издания книги в заданном диапазоне
inline auto YearBetween(int from, int to) {
    return [from, to](const Book &book) { return (book.GetYear() >= from) && (book.GetYear() <= to); };
}

// === Композиции предикатов (на основе fold expressions) ===

// Композиция "логическое И"
template <typename... Preds>
auto all_of(Preds... preds) {
    return [preds...](const Book &book) { return (preds(book) && ...); };
}

// Композиция "логическое ИЛИ"
template <typename... Preds>
auto any_of(Preds... preds) {
    return [preds...](const Book &book) { return (preds(book) || ...); };
}

// === Фильтрация книг ===

// Шаблонная функция фильтрации диапазона книг из картотеки по заданному предикату
template <BookIterator It, BookSentinel<It> Sen, BookPredicate Pred>
std::vector<std::reference_wrapper<const Book>> filterBooks(It first, Sen last, Pred pred) {
    using RefBook = std::reference_wrapper<const Book>;  // псевдоним для типа ссылки на книгу

    std::vector<RefBook> out_books;  // создаем выходной массив ссылок на отобранные книги

    // Копируем в выходной массив только те книги, которые удовлетворяют предикату
    std::copy_if(first, last, std::back_inserter(out_books), [&pred](const Book &book) { return pred(book); });

    return out_books;  // работает оптимизация компилятора RVO (отсутствует копирование)
}

}  // namespace bookdb