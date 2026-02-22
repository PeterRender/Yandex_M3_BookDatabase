#pragma once

#include <format>
#include <stdexcept>
#include <string_view>

namespace bookdb {

// Перечисляемый тип жанра книги
enum class Genre { Fiction, NonFiction, SciFi, Biography, Mystery, Unknown };

// Функция, преобразующая строковое значение жанра в его перечисляемый тип
// (может работать во t компиляции, если аргумент const)
constexpr Genre GenreFromString(std::string_view s) {
    // clang-format off
    if (s == "Fiction")     return Genre::Fiction;
    if (s == "NonFiction")  return Genre::NonFiction;
    if (s == "SciFi")       return Genre::SciFi;
    if (s == "Biography")   return Genre::Biography;
    if (s == "Mystery")     return Genre::Mystery;
    // clang-format on
    return Genre::Unknown;
}

// Класс описателя книги
struct Book {
    // string_view для экономии памяти, чтобы ссылаться на оригинальную строку, хранящуюся в другом контейнере
    std::string_view author_;
    std::string title_;

    int year_;
    Genre genre_;
    double rating_;
    int read_count_;

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
        std::string genre_str;

        // clang-format off
        using bookdb::Genre;
        switch (g) {
            case Genre::Fiction:    genre_str = "Fiction"; break;
            case Genre::Mystery:    genre_str = "Mystery"; break;
            case Genre::NonFiction: genre_str = "NonFiction"; break;
            case Genre::SciFi:      genre_str = "SciFi"; break;
            case Genre::Biography:  genre_str = "Biography"; break;
            case Genre::Unknown:    genre_str = "Unknown"; break;
            default:
                throw logic_error{"Unsupported bookdb::Genre"};
            }
        // clang-format on
        // Берем строку жанра и копируем ее как есть в выходной буфер.
        return format_to(fc.out(), "{}", genre_str);
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
        // "Title: "{}", author: {}, year: {}, genre: {}, rating: {}/5.0, read {} times"
        return format_to(ctx.out(), "Title: \"{}\", author: {}, year: {}, genre: {}, rating: {:.1f}/5.0, read {} times",
                         book.title_, book.author_, book.year_,
                         book.genre_,  // здесь сработает специализация std::formatter для Genre
                         book.rating_, book.read_count_);
    }

    // Метод, выполняющий парсинг спецификаторов формата (значения в скобках {})
    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // игнорируем спецификаторы и возвращаем начало строки формата
    }
};

}  // namespace std
