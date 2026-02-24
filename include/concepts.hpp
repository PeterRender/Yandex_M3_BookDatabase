#pragma once

#include <concepts>
#include <iterator>

#include "book.hpp"

namespace bookdb {

// Концепт для контейнера, хранящего описатели книг
template <typename T>
concept BookContainerLike = requires(T t) {
    typename T::value_type;                               // должен иметь внутренний тип
    requires std::same_as<typename T::value_type, Book>;  // внутренний тип должен быть Book
    typename T::const_iterator;                           // должен иметь константный итератор

    // Должен предоставлять доступ только для чтения к своему началу и концу
    { t.cbegin() } -> std::same_as<typename T::const_iterator>;
    { t.cend() } -> std::same_as<typename T::const_iterator>;

    { t.size() } -> std::convertible_to<std::size_t>;  // должен возвращать свой размер
};

// Концепт для итератора по описателям книг
template <typename T>  //
concept BookIterator = std::input_iterator<T> && requires(T it, const Book &book) {
    // BookIterator должен отвечать стандартному концепту итератора ввода (чтобы читать, двигаться вперед)
    // и удовлетворять следующим требованиям:
    { *it } -> std::convertible_to<const Book &>;  // должен уметь разыменовываться в const Book&
    requires std::same_as<typename std::iterator_traits<T>::value_type, Book>;  // внутренний тип должен быть Book
};

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