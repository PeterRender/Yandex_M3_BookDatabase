#pragma once

#include <print>
#include <string>
#include <string_view>
#include <unordered_set>  // подключение std::unordered_set
#include <vector>

#include "book.hpp"                  // заголовочный файл с классом описателя книги и шаблонами для его форматирования
#include "concepts.hpp"              // концепты для работы с описателями книг
#include "heterogeneous_lookup.hpp"  // прозрачные компараторы и хэш-функция для гетерогенного поиска

namespace bookdb {

// Шаблонный класс-контейнер картотеки книг
template <BookContainerLike BookContainer = std::vector<Book>>
class BookDatabase {
public:
    // Псевдонимы типов, совместимые со стандартными алгоритмами
    using value_type = Book;
    using size_type = typename BookContainer::size_type;
    using difference_type = typename BookContainer::difference_type;

    using reference = value_type &;
    using const_reference = const value_type &;

    using iterator = typename BookContainer::iterator;
    using const_iterator = typename BookContainer::const_iterator;

    // Контейнер для авторов с поддержкой гетерогенного поиска
    // (используем std::string для хранения авторов, т.к. в описателе книги автор задан в виде std::string_view)
    using AuthorContainer = std::unordered_set<std::string, TransparentStringHash, TransparentStringEqual>;

    // Конструктор по умолчанию
    BookDatabase() = default;

    // Конструктор, принимающий список инициализации
    BookDatabase(std::initializer_list<Book> books) {
        for (const auto &book : books) {
            PushBack(book);
        }
    }

    // Метод очистки картотеки книг
    void Clear() {
        books_.clear();
        authors_.clear();
    }

    // Стандартные методы доступа к итераторам контейнера
    iterator begin() { return books_.begin(); }
    iterator end() { return books_.end(); }
    const_iterator begin() const { return books_.begin(); }
    const_iterator end() const { return books_.end(); }
    const_iterator cbegin() const { return books_.cbegin(); }
    const_iterator cend() const { return books_.cend(); }

    // Стандартные методы доступа к данным о размере контейнера
    size_type size() const { return books_.size(); }
    bool empty() const { return books_.empty(); }

    // Метод, добавляющий описатель книги в картотеку
    void PushBack(const Book &book) {
        books_.push_back(book);                      // добавляем описатель книги в хранилище описателей
        authors_.insert(std::string(book.author_));  // добавляем автора книги в хранилище авторов
    }

    // Шаблонный метод, принимающий аргументы для создания описателя книги прямо внутри картотеки
    template <typename... Args>
    decltype(auto) EmplaceBack(Args &&...args) {
        auto &book = books_.emplace_back(std::forward<Args>(args)...);  // cоздаем описатель книги прямо в хранилище
        authors_.insert(std::string(book.author_));                     // добавляем автора книги в хранилище авторов
        return book;
    }

    // Методы доступа "только для чтения" к хранилищам описателей и авторов книг
    const BookContainer &GetBooks() const { return books_; }
    const AuthorContainer &GetAuthors() const { return authors_; }

private:
    BookContainer books_;      // хранилище описателей книг
    AuthorContainer authors_;  // хранилище авторов книг
};

}  // namespace bookdb

namespace std {
// Специализация шаблона std::formatter для типа BookDatabase<std::vector<Book>>
// (позволяет использовать std::format и std::print с BookDatabase)
template <>
struct formatter<bookdb::BookDatabase<std::vector<bookdb::Book>>> {
    // Шаблонный метод-инструкция, отвечающий за преобразование значения BookDatabase<std::vector<Book>> в строку
    // (параметр FormatContext задает, куда выводить результат)
    template <typename FormatContext>
    auto format(const bookdb::BookDatabase<std::vector<bookdb::Book>> &db, FormatContext &fc) const {
        // Выводим заголовок, содержащий размер картотеки
        format_to(fc.out(), "BookDatabase (size = {}): ", db.size());

        // Выводим все описатели книг, хранящиеся в картотеке
        format_to(fc.out(), "Books:\n");
        for (const auto &book : db.GetBooks()) {
            format_to(fc.out(), "- {}\n", book);
        }

        // Выводим всех авторов книг, которые имеются в картотеке
        format_to(fc.out(), "Authors:\n");
        for (const auto &author : db.GetAuthors()) {
            format_to(fc.out(), "- {}\n", author);
        }

        return fc.out();
    }

    // Метод, выполняющий парсинг спецификаторов формата (значения в скобках {})
    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // игнорируем спецификаторы и возвращаем начало строки формата
    }
};
}  // namespace std
