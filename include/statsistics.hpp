#pragma once

#include <algorithm>   // подключение std::accumulate, std::sample и др.
#include <flat_map>    // подключение плоского ассоциативного контейнера (С++23)
#include <functional>  // подключение std::reference_wrapper
#include <iterator>
#include <random>  // подключение ГПСЧ std::random_device, std::mt19937

#include "book_database.hpp"

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

    return histogram;  // работает оптимизация компилятора RVO (отсутствует копирование)
}

// Шаблонная функция расчета средних рейтингов книг по жанрам
// (немодифицируемость входных данных гарантируется концептом BookIterator)
template <BookIterator It, BookSentinel<It> Sen>
auto calculateGenreRatings(It first, Sen last) {
    // Для расчета средних рейтингов используем std::flat_map с элементами
    // вида (жанр, {средний рейтинг, число книг жанра})
    std::flat_map<Genre, std::pair<double, size_t>> avg_map;

    // Вычисляем средние рейтинги книг по жанрам за один проход
    std::for_each(first, last, [&avg_map](const Book &book) {
        // Пытаемся вставить новый элемент (жанр, {рейтинг, 1 книга})
        auto [map_it, inserted] = avg_map.try_emplace(book.genre_, std::pair{book.rating_, 1});

        // Если жанр уже есть в контейнере, то обновляем для него средний рейтинг
        if (!inserted) {
            map_it->second.second++;  // увеличиваем счетчик книг
            // Обновляем среднее по формуле: avg_new = avg_old + (x - avg_old) / (n + 1)
            map_it->second.first += (book.rating_ - map_it->second.first) / map_it->second.second;
        }
    });

    return avg_map;  // работает оптимизация компилятора RVO (отсутствует копирование)
}

// Шаблонная функция расчета среднего рейтинга всех книг в картотеке
template <BookContainerLike T>
double calculateAverageRating(const BookDatabase<T> &db) {
    // Если картотека пуста, возвращаем 0.0
    if (db.empty()) {
        return 0.0;
    }

    // Суммирование рейтингов выполняем с помощью std::accumulate
    double sum = std::accumulate(db.begin(),                         // начало диапазона
                                 db.end(),                           // конец диапазона
                                 0.0,                                // начальное значение
                                 [](double acc, const Book &book) {  // бинарная операция (сложение рейтингов)
                                     return acc + book.rating_;
                                 });

    // Версия через std::transform_reduce с рапараллеливанием работает медленнее из-за оверхеда на потоки
    // (политика std::execution::par_unseq - "делай как можно быстрее": для маленьких данных - последовательно, для
    // больших - компилятор/библиотека решают, стоит ли распараллеливать)
    // double sum = std::transform_reduce(std::execution::par_unseq, db.begin(), db.end(), 0.0,
    //                                    std::plus<>(),          // бинарная операция для reduce (сложение рейтингов)
    //                                    [](const Book &book) {  // унарная операция для transform (извлечение
    //                                    рейтинга)
    //                                        return book.rating_;
    //                                    });

    return sum / db.size();  // возвращаем средний рейтинг
}

// Шаблонная функция случайной выборки заданного числа книг из картотеки
template <BookContainerLike T>
std::vector<std::reference_wrapper<const Book>> sampleRandomBooks(const BookDatabase<T> &db, size_t count) {
    using RefBook = std::reference_wrapper<const Book>;  // псевдоним для типа ссылки на книгу

    size_t n = std::min(count, db.size());  // ограничиваем число выбираемых книг размером картотеки
    std::vector<RefBook> out_books;         // создаем массив ссылок на отобранные книги
    out_books.reserve(n);                   // резервируем память под n элементов выборки

    // Если запрошено 0 книг (или картотека пуста), то возвращаем пустую выборку
    if (n == 0) {
        return out_books;
    }

    // Выполняем псевдослучайную выборку книг из картотеки с помощью std::sample
    std::sample(db.begin(), db.end(),                   // входной диапазон
                std::back_inserter(out_books),          // выходной итератор
                n,                                      // число выбираемых книг
                std::mt19937{std::random_device{}()});  // генератор псевдослучайных чисел

    return out_books;  // работает оптимизация компилятора RVO (отсутствует копирование)
}

// Шаблонная функция для получения топ-N книг по заданному критерию
// (функции разрешено изменять порядок книг во входном контейнере (картотеке книг))
template <BookContainerLike T, typename Comparator>
std::vector<std::reference_wrapper<const Book>> getTopNBy(BookDatabase<T> &db, size_t n, Comparator comp) {
    using RefBook = std::reference_wrapper<const Book>;  // псевдоним для типа ссылки на книгу

    size_t num_tops = std::min(n, db.size());  // ограничиваем топ-N книг размером картотеки
    std::vector<RefBook> top_result;           // создаем массив ссылок отобранные на топ-N книг
    top_result.reserve(num_tops);              // резервируем память под топ-N книг

    // Если запрошено 0 книг (или картотека пуста), то возвращаем пустую подборку
    if (n == 0) {
        return top_result;
    }

    // Частично сортируем картотеку с помощью std::partial_sort - сложность O(nlog(num_tops))
    // (первые num_tops элементов будут упорядочены согласно заданному критерию)
    std::partial_sort(db.begin(), db.begin() + num_tops, db.end(), comp);

    // Копируем ссылки на топ-N книг в выходной массив ссылок
    std::copy(db.begin(), db.begin() + num_tops, std::back_inserter(top_result));

    return top_result;  // работает оптимизация компилятора RVO (отсутствует копирование)
}

}  // namespace bookdb
