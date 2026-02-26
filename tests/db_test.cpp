#include <gtest/gtest.h>
#include <unordered_set>  // подключение std::unordered_set

#include "book.hpp"                  // заголовочный файл с классом описателя книги и шаблонами для его форматирования
#include "book_database.hpp"         // заголовочный файл с классом картотеки книг и шаблоном ее форматирования
#include "comparators.hpp"           // компараторы для работы с описателями книг
#include "concepts.hpp"              // концепты для работы с описателями книг
#include "heterogeneous_lookup.hpp"  // прозрачные компараторы и хэш-функция для гетерогенного поиска
#include "statsistics.hpp"           // подключение функций расчета статистик картотеки книг

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

// Тест корректного форматрирования описателя книги
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

    Book hobbit("The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203);
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

// Тест корректности работы концептов
TEST(BookTest, CorrectConcepts) {
    using namespace bookdb;
    using namespace bookdb::comp;

    // Контейнер std::vector должен отвечать концепту BookContainerLike
    static_assert(BookContainerLike<std::vector<Book>>);

    // Итератор std::vector должен отвечать концепту BookIterator
    static_assert(BookIterator<std::vector<Book>::const_iterator>);

    // Компараторы описателей книг должны отвечать концепту BookComparator
    static_assert(AllComparatorsValid<LessByAuthor, LessByTitle, LessByYear, LessByRating, LessByReadCount,
                                      GreaterByYear, GreaterByRating, GreaterByReadCount>);

    // Лямбда должна отвечать концепту BookPredicate
    auto isSciFi = [](const Book &b) { return b.genre_ == Genre::SciFi; };
    static_assert(BookPredicate<decltype(isSciFi)>);

    // Проверяем некорретные случаи
    static_assert(!BookContainerLike<std::string>);
    static_assert(!BookIterator<int>);
    static_assert(!BookPredicate<int>);

    SUCCEED() << "All concepts checks passed";
}

// Тест корректности работы гетерогенного поиска
TEST(BookTest, HeteroLookup) {
    using namespace bookdb;

    // Создаем тестовые контейнеры с поддержкой гетерогенного поиска
    std::map<std::string, int, TransparentStringLess> authors_map{{"Fitzgerald", 2}, {"Tolkien", 1}, {"Huxley", 3}};
    std::unordered_set<std::string, TransparentStringHash, TransparentStringEqual> authors_umap{"Tolkien", "Fitzgerald",
                                                                                                "Huxley"};

    // Шаблонная лямбда для проверки пакета ключей разных типов (const char*, std::string и std::string_view)
    auto test_find = []<typename Container, typename... Keys>(Container &cont, Keys &&...keys) {
        return ((cont.find(std::forward<Keys>(keys)) != cont.end()) && ...);  // fold expression для обработки пакета.
    };

    // Проверка поиска по существующим ключам (трех типов)
    EXPECT_TRUE(test_find(authors_map,
                          "Fitzgerald",               // const char*
                          std::string("Tolkien"),     // std::string
                          std::string_view("Huxley")  // std::string_view
                          ));
    EXPECT_TRUE(test_find(authors_umap,
                          "Tolkien",                  // const char*
                          std::string("Fitzgerald"),  // std::string
                          std::string_view("Huxley")  // std::string_view
                          ));

    // Проверка поиска по несуществующим ключам (трех типов)
    EXPECT_FALSE(test_find(authors_map, "Salinger", std::string("Lee"), std::string_view("Orwell")));
    EXPECT_FALSE(test_find(authors_umap, "Salinger", std::string("Lee"), std::string_view("Orwell")));
}

// Тест корректности заполнения картотеки книг
TEST(BookDatabaseTest, CorrectFilling) {
    using namespace bookdb;

    // Создаем тестовый массив описателей книг
    std::vector<Book> books = {{"1984", "George Orwell", 1949, Genre::SciFi, 4., 190},
                               {"Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143},
                               {"The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203},
                               {"The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120},
                               {"Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98}};

    // Создаем тестовые объекты картотек (исходно они должны быть пустыми)
    BookDatabase<std::vector<Book>> bookDbs[2];
    for (const auto &db : bookDbs) {
        EXPECT_EQ(db.size(), 0);
        EXPECT_TRUE(db.empty());
    }

    // Заполняем картотеки описателями книг (1-ую картотеку - путем добавления, 2-ую - путем создания внутри)
    for (const auto &b : books) {
        bookDbs[0].PushBack(b);
        auto &empl_book = bookDbs[1].EmplaceBack(b.title_, b.author_, b.year_, b.genre_, b.rating_, b.read_count_);
        EXPECT_EQ(empl_book.title_, b.title_);  // должна вернуться ссылка на созданную книгу
    }

    // Проверяем заполненные картотеки
    for (const auto &db : bookDbs) {
        // Размеры картотеки должны корректно измениться
        EXPECT_EQ(db.size(), books.size());
        EXPECT_FALSE(db.empty());

        // Число авторов в картотеке должно равняться числу уникальных (неповторяющихся) авторов
        EXPECT_EQ(db.GetAuthors().size(), 4);

        // В картотеке должны быть все добавленные уникальные авторы
        for (const auto &b : books) {
            EXPECT_TRUE(db.GetAuthors().contains(b.author_));
        }
    }
}

// Тест корректности инициализации картотеки книг
TEST(BookDatabaseTest, CorrectInit) {
    using namespace bookdb;

    // Создаем список описателей книг
    std::initializer_list<Book> book_list = {{"1984", "George Orwell", 1949, Genre::SciFi, 4., 190},
                                             {"Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143},
                                             {"The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203}};

    // Инициализируем картотеку списком описателей книг
    BookDatabase db{book_list};

    EXPECT_EQ(db.size(), book_list.size());  // размер картотеки должен совпадать с размером списка описателей книг
    EXPECT_EQ(db.GetAuthors().size(), 2);    // в картотеке должны быть только уникальные авторы

    // В картотеке должны быть все добавленные уникальные авторы
    for (const auto &b : book_list) {
        EXPECT_TRUE(db.GetAuthors().contains(b.author_));
    }
}

// Тест совместимости со стандартными алгоритмами
TEST(BookDatabaseTest, StdAlgoReadiness) {
    using namespace bookdb;
    using namespace bookdb::comp;

    // Инициализируем картотеку тестовым списком книг
    BookDatabase db = {
        {"The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120},
        {"1984", "George Orwell", 1949, Genre::SciFi, 4.0, 190},
        {"Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98},
        {"Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143},
        {"The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203},
    };

    // Выполняем сортировку книг по убыванию рейтинга с сохранением относительного порядка
    std::stable_sort(db.begin(), db.end(), GreaterByRating{});

    // Проверяем результат стабильной сортировки по рейтингу
    auto it = db.begin();
    EXPECT_EQ(it->title_, "The Hobbit");            // на 1-ом месте должен быть "The Hobbit"
    EXPECT_EQ((++it)->title_, "The Great Gatsby");  // на 2-ом месте - "The Great Gatsby"
    EXPECT_EQ((++it)->title_, "Brave New World");   // на 3-м месте - "Brave New World"
    EXPECT_EQ((++it)->title_, "Animal Farm");       // на 4-м месте - "Animal Farm"
    EXPECT_EQ((++it)->title_, "1984");              // на 5-м месте - "1984"

    // Проверяем поиск по условию
    auto hobbit_it = std::find_if(db.begin(), db.end(), [](const Book &b) { return b.year_ == 1937; });
    EXPECT_NE(hobbit_it, db.end());
}

// Тест функции построения гистограммы авторов
TEST(StatisticsTest, AuthorHistogram) {
    using namespace bookdb;

    // Создаем тестовую картотеку книг с повторяющимися авторами
    BookDatabase<std::vector<Book>> db = {{"The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203},
                                          {"The Silmarillion", "J.R.R. Tolkien", 1977, Genre::Fiction, 4.2, 56},
                                          {"1984", "George Orwell", 1949, Genre::SciFi, 4.0, 190},
                                          {"Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143},
                                          {"Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98},
                                          {"The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120}};

    auto histogram = buildAuthorHistogramFlat(db);

    // Проверяем количество уникальных авторов
    EXPECT_EQ(histogram.size(), 4);  // Tolkien, Orwell, Huxley, Fitzgerald

    // Проверяем количество книг каждого автора
    EXPECT_EQ(histogram["J.R.R. Tolkien"], 2);
    EXPECT_EQ(histogram["George Orwell"], 2);
    EXPECT_EQ(histogram["Aldous Huxley"], 1);
    EXPECT_EQ(histogram["F. Scott Fitzgerald"], 1);

    // Проверяем автора, которого нет в картотеке
    EXPECT_EQ(histogram.contains("J.D. Salinger"), false);
}

// Тест функции расчета средних рейтингов книг по жанрам
TEST(StatisticsTest, CalculateGenreRatings) {
    using namespace bookdb;

    // Создаем тестовую картотеку книг
    BookDatabase db = {
        {"The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120},
        {"1984", "George Orwell", 1949, Genre::SciFi, 4.0, 190},
        {"Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98},
        {"Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143},
        {"The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203},
    };

    // Вычисляем средние рейтинги книг по жанрам
    auto ratings = calculateGenreRatings(db.begin(), db.end());

    // Должно быть 2 жанра: Fiction и SciFi
    EXPECT_EQ(ratings.size(), 2);

    // Проверяем средний рейтинг по Fiction: (4.5 + 4.4 + 4.9) / 3 = 4.6
    EXPECT_NEAR(ratings[Genre::Fiction].first, 4.6, 0.01);
    EXPECT_EQ(ratings[Genre::Fiction].second, 3);  // должно быть 3 книги этого жанра

    // Проверяем средний рейтинг по SciFi: (4.0 + 4.5) / 2 = 4.25
    EXPECT_NEAR(ratings[Genre::SciFi].first, 4.25, 0.01);
    EXPECT_EQ(ratings[Genre::SciFi].second, 2);  // должно быть 2 книги этого жанра
}