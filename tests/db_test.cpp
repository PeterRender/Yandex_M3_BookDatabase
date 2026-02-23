#include <gtest/gtest.h>

#include "book.hpp"
#include "comparators.hpp"

using namespace bookdb;

// Тест создания корректного описателя книги с заданием жанра с помощью строки
TEST(BookTest, BookWithStrGenre) {
    Book book("The Hobbit", "J.R.R. Tolkien", 1937, "Fiction", 4.9, 203);

    EXPECT_EQ(book.title_, "The Hobbit");
    EXPECT_EQ(book.author_, "J.R.R. Tolkien");
    EXPECT_EQ(book.year_, 1937);
    EXPECT_EQ(book.genre_, Genre::Fiction);
    EXPECT_DOUBLE_EQ(book.rating_, 4.9);
    EXPECT_EQ(book.read_count_, 203);
}

// Тест создания корректного описателя книги с заданием жанра с помощью перечисляемой константы
TEST(BookTest, BookWithEnumGenre) {
    Book book("Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98);

    EXPECT_EQ(book.title_, "Brave New World");
    EXPECT_EQ(book.author_, "Aldous Huxley");
    EXPECT_EQ(book.year_, 1932);
    EXPECT_EQ(book.genre_, Genre::SciFi);
    EXPECT_DOUBLE_EQ(book.rating_, 4.5);
    EXPECT_EQ(book.read_count_, 98);
}

// Тест корректного преобразования описателя книги в строку
TEST(BookTest, FormatBook) {
    Book book("The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120);
    std::string result = std::format("{}", book);

    // Выходная строка должна содержать все ожидаемые части
    EXPECT_NE(result.find("The Great Gatsby"), std::string::npos);
    EXPECT_NE(result.find("F. Scott Fitzgerald"), std::string::npos);
    EXPECT_NE(result.find("1925"), std::string::npos);
    EXPECT_NE(result.find("Fiction"), std::string::npos);
    EXPECT_NE(result.find("4.5"), std::string::npos);
    EXPECT_NE(result.find("120"), std::string::npos);
}

// Тест корректного сравнения описателей книг (включая гетерогенные версии)
TEST(BookTest, CorrectComparators) {
    using namespace bookdb;
    using namespace bookdb::comp;

    Book hobbit("The Hobbit", "Tolkien", 1937, Genre::Fiction, 4.9, 203);
    Book gatsby("The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120);
    Book world("Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98);

    // Определяем структуру тест-кейса
    struct TestCase {
        std::string_view name;                                       // название теста
        const Book &lhs;                                             // левая книга
        const Book &rhs;                                             // правая книга
        std::function<bool(const Book &, const Book &)> comparator;  // указатель на лямбду-компаратор
        bool expected;                                               // ожидаемое значение компаратора
    };

    // Лямбды-обертки тестируемых прозрачных компараторов
    auto LessByAuthorComp = [](const Book &a, const Book &b) {
        return LessByAuthor{}(a, b) && LessByAuthor{}(a, b.author_) && LessByAuthor{}(a.author_, b);
    };
    auto LessByTitleComp = [](const Book &a, const Book &b) {
        return LessByTitle{}(a, b) && LessByTitle{}(a, b.title_) && LessByTitle{}(a.title_, b);
    };
    auto LessByYearComp = [](const Book &a, const Book &b) {
        return LessByYear{}(a, b) && LessByYear{}(a, b.year_) && LessByYear{}(a.year_, b);
    };
    auto LessByRatingComp = [](const Book &a, const Book &b) {
        return LessByRating{}(a, b) && LessByRating{}(a, b.rating_) && LessByRating{}(a.rating_, b);
    };
    auto LessByReadCountComp = [](const Book &a, const Book &b) {
        return LessByReadCount{}(a, b) && LessByReadCount{}(a, b.read_count_) && LessByReadCount{}(a.read_count_, b);
    };
    auto GreaterByYearComp = [](const Book &a, const Book &b) {
        return GreaterByYear{}(a, b) && GreaterByYear{}(a, b.year_) && GreaterByYear{}(a.year_, b);
    };
    auto GreaterByRatingComp = [](const Book &a, const Book &b) {
        return GreaterByRating{}(a, b) && GreaterByRating{}(a, b.rating_) && GreaterByRating{}(a.rating_, b);
    };
    auto GreaterByReadCountComp = [](const Book &a, const Book &b) {
        return GreaterByReadCount{}(a, b) && GreaterByReadCount{}(a, b.read_count_) &&
               GreaterByReadCount{}(a.read_count_, b);
    };

    // Создаем массив тестов для пар книг
    std::vector<TestCase> tests = {{"LessByAuthor", gatsby, hobbit, LessByAuthorComp, true},
                                   {"LessByTitle", gatsby, hobbit, LessByTitleComp, true},
                                   {"LessByYear", gatsby, hobbit, LessByYearComp, true},
                                   {"LessByRating", gatsby, hobbit, LessByRatingComp, true},
                                   {"LessByReadCount", gatsby, hobbit, LessByReadCountComp, true},
                                   {"GreaterByYear", world, gatsby, GreaterByYearComp, true},
                                   {"GreaterByRating", world, gatsby, GreaterByRatingComp, false},
                                   {"GreaterByReadCount", world, gatsby, GreaterByReadCountComp, false}};

    // Запускаем все тесты в цикле
    for (const auto &[name, lhs, rhs, comp, expected] : tests) {
        EXPECT_EQ(comp(lhs, rhs), expected) << "Failed: " << name;
    }
}
