#pragma once

#include <concepts>
#include <iterator>

#include "book.hpp"

namespace bookdb {

// Концепт для контейнера, хранящего описатели книг
template <typename T>
concept BookContainerLike = std::ranges::contiguous_range<T> &&            // данные хранятся в памяти подряд
                            std::same_as<typename T::value_type, Book> &&  // хранит описатели книг
                            requires(T t, const T ct) {
                                // Явно проверяем нужные итераторы и методы
                                typename T::const_iterator;
                                { ct.cbegin() } -> std::same_as<typename T::const_iterator>;  // есть const begin
                                { ct.cend() } -> std::same_as<typename T::const_iterator>;    // есть const end
                                { t[0] } -> std::convertible_to<const Book &>;                // есть operator[]
                                t.push_back(std::declval<Book>());  // можно добавлять элементы в конец
                            };

// Концепт для итератора по описателям книг
template <typename T>
concept BookIterator = std::contiguous_iterator<T> &&  // итератор на непрерывную память (для сортировки)
                       std::same_as<typename std::iter_value_t<T>, Book>;  // итератор на книги

// Концепт для ограничителя диапазона
template <typename S, typename I>
concept BookSentinel = std::sentinel_for<S, I>;  // должен отвечать стандартному концепту ограничителя S для итератора T

// Концепт для предиката, работающего с описателями книг
// Используем стандартный std::predicate, который требует:
// - вызываемости с const Book&
// - возврата bool результата
// - неизменяемости внутреннего состояния
template <typename P>
concept BookPredicate = std::predicate<P, const Book &>;

// Концепт для компаратора описателей книг с поддержкой гетерогенного поиска
template <typename C>
concept BookComparator = std::relation<C, const Book &, const Book &> && requires {
    // BookComparator должен отвечать стандартному концепту отношения для сравнения двух описателей книг
    // и требованию прозрачных компараторов (маркер is_transparent должен быть void):
    requires !requires { typename C::is_transparent; } || std::same_as<typename C::is_transparent, void>;
};

// Вспомогательный концепт для проверки пакета компараторов
template <typename... Ts>
concept AllComparatorsValid = (BookComparator<Ts> && ...);

}  // namespace bookdb