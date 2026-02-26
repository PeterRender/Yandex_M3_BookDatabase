#pragma once

#include <algorithm>
#include <flat_map>  // подключение плоского ассоциативного контейнера (С++23)
#include <iterator>
#include <random>
#include <stdexcept>
#include <string_view>

#include "book_database.hpp"

#include <print>

namespace bookdb {

// Шаблонная функция построения гистограммы количества книг по авторам
template <BookContainerLike T, typename Comparator = TransparentStringLess>
auto buildAuthorHistogramFlat(const BookDatabase<T> &cont, Comparator comp = {}) {
    // Для построения гистограммы используем адаптер std::flat_map:
    // а) владеющий строками-авторами (во избежание образования висячих ссылок при передаче наружу)
    // б) с прозрачным компаратором (во избежание создания временных объектов std::string при поиске элементов)
    std::flat_map<std::string, size_t, Comparator> histogram;

    // Цикл по всем книгам в картотеке
    for (const auto &book : cont.GetBooks()) {
        // Пытаемся вставить в контейнер новый элемент (автор, 1 книга)
        auto [it, inserted] = histogram.try_emplace(std::string(book.author_), 1);

        // Если элемент не был вставлен (уже есть в контейнере), то увеличиваем счетчик книг для автора
        if (!inserted) {
            it->second++;
        }
    }

    return histogram;  // работает оптимизация компилятора RVO
}

// Шаблонная функция расчета средних рейтингов книг по жанрам
// (немодифицируемость входных данных гарантируется концептом BookIterator)
template <BookIterator It, BookSentinel<It> Sen>
auto calculateGenreRatings(It first, Sen last) {
    // Для расчета средних рейтингов используем std::flat_map с элементами
    // вида (жанр, {средний рейтинг, число книг жанра})
    std::flat_map<Genre, std::pair<double, size_t>> avg_map;

    // Вычисляем средние рейтинги книг по жанрам за один проход
    for (auto it = first; it != last; ++it) {
        const auto &book = *it;

        // Пытаемся вставить новый элемент (жанр, {рейтинг, 1 книга})
        auto [map_it, inserted] = avg_map.try_emplace(book.genre_, std::pair{book.rating_, 1});

        // Если жанр уже есть в контейнере, то обновляем для него средний рейтинг
        if (!inserted) {
            map_it->second.second++;  // увеличиваем счетчик книг
            // Обновляем среднее по формуле: avg_new = avg_old + (x - avg_old) / (n + 1)
            map_it->second.first += (book.rating_ - map_it->second.first) / map_it->second.second;
        }
    }

    return avg_map;  // работает оптимизация компилятора RVO
}
}  // namespace bookdb
