#include <gtest/gtest.h>

#include "book.hpp"

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
