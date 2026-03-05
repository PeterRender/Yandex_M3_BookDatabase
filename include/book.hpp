#pragma once

#include <format>
#include <stdexcept>
#include <string_view>

namespace bookdb {

// Перечисляемый тип жанра книги
enum class Genre { Fiction, NonFiction, SciFi, Biography, Mystery, Unknown };

using namespace std::string_view_literals;  // подключение оператора sv

// Массив пар {номер жанра, имя жанра}
constexpr std::array genres = {std::pair{Genre::Fiction, "Fiction"sv}, std::pair{Genre::NonFiction, "NonFiction"sv},
                               std::pair{Genre::SciFi, "SciFi"sv},     std::pair{Genre::Biography, "Biography"sv},
                               std::pair{Genre::Mystery, "Mystery"sv}, std::pair{Genre::Unknown, "Unknown"sv}};

// Функция, преобразующая строковое значение жанра в его перечисляемый тип
// (может работать во t компиляции, если аргумент const)
constexpr Genre GenreFromString(std::string_view s) {
    for (const auto &[genre_id, genre_name] : genres) {
        if (genre_name == s)
            return genre_id;
    }
    return Genre::Unknown;
}

// Класс описателя книги
class Book {
public:
    // Конструктор описателя книги, принимающий жанр в виде строки
    // (может работать во t компиляции, если вcе аргументы const)
    constexpr Book(std::string_view title, std::string_view author, int year, std::string_view genre_str, double rating,
                   int read_count)
        : author_(author), title_(title), year_(year), genre_(GenreFromString(genre_str)), rating_(rating),
          read_count_(read_count) {}

    // Конструктор описателя книги, принимающий жанр в виде перечисляемого типа
    // (может работать во t компиляции, если вcе аргументы const)
    constexpr Book(std::string_view title, std::string_view author, int year, Genre genre, double rating,
                   int read_count)
        : author_(author), title_(title), year_(year), genre_(genre), rating_(rating), read_count_(read_count) {}

    // Методы доступа к данным членам класса (могут работать во t компиляции)
    constexpr std::string_view GetAuthor() const { return author_; }
    constexpr const std::string &GetTitle() const { return title_; }
    constexpr int GetYear() const { return year_; }
    constexpr Genre GetGenre() const { return genre_; }
    constexpr double GetRating() const { return rating_; }
    constexpr int GetReadCount() const { return read_count_; }

private:
    // string_view для экономии памяти, чтобы ссылаться на оригинальную строку, хранящуюся в другом контейнере
    std::string_view author_;  // автор
    std::string title_;        // название книги
    int year_;                 // год издания
    Genre genre_;              // жанр
    double rating_;            // рейтинг (max - 5.0)
    int read_count_;           // число прочтений
};
}  // namespace bookdb

namespace std {

// Специализация шаблона std::formatter для типа bookdb::Genre
// (позволяет использовать std::format и std::print с жанрами книг)
template <>
struct formatter<bookdb::Genre, char> {
    // Шаблонный метод-инструкция, отвечающий за преобразование значения Genre в строку
    // (параметр FormatContext задает, куда выводить результат)
    template <typename FormatContext>
    auto format(const bookdb::Genre g, FormatContext &fc) const {
        // Преобразуем номер жанра в индекс в массиве genres (enum совпадает с порядком в массиве genres)
        size_t idx = static_cast<size_t>(g);

        // Проверяем, что индекс в допустимых пределах
        if (idx >= bookdb::genres.size()) {
            throw std::logic_error{"Unsupported bookdb::Genre"};
        }

        // Берем строку жанра и копируем ее как есть в выходной буфер.
        return format_to(fc.out(), "{}", bookdb::genres[idx].second);
    }

    // Метод, выполняющий парсинг спецификаторов формата (значения в скобках {})
    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // игнорируем спецификаторы и возвращаем начало строки формата
    }
};

// Специализация шаблона std::formatter для типа bookdb::Book
// (позволяет использовать std::format и std::print с объектами книг)
template <>
struct formatter<bookdb::Book, char> {
    // Шаблонный метод-инструкция, отвечающий за преобразование типа Book в строку
    // (параметр FormatContext задает, куда выводить результат)
    template <typename FormatContext>
    auto format(const bookdb::Book &book, FormatContext &ctx) const {
        // Преобразуем описатель книги в строку вида:
        // ""{title}" by {author} ({year}) [{genre}], {rating}/5.0, {} reads"
        return format_to(ctx.out(), "\"{}\" by {} ({}) [{}], {:.1f}/5.0, {} reads", book.GetTitle(), book.GetAuthor(),
                         book.GetYear(),
                         book.GetGenre(),  // здесь сработает специализация std::formatter для Genre
                         book.GetRating(), book.GetReadCount());
    }

    // Метод, выполняющий парсинг спецификаторов формата (значения в скобках {})
    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // игнорируем спецификаторы и возвращаем начало строки формата
    }
};

}  // namespace std
