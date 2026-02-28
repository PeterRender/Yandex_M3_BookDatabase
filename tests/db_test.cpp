#include <gtest/gtest.h>

#include <deque>  // подключение std::deque
#include <list>   // подключение std::list

#include "book.hpp"                  // заголовочный файл с классом описателя книги и шаблонами для его форматирования
#include "book_database.hpp"         // заголовочный файл с классом картотеки книг и шаблоном ее форматирования
#include "comparators.hpp"           // компараторы для работы с описателями книг
#include "concepts.hpp"              // концепты для работы с описателями книг
#include "filters.hpp"               // фильтры для работы с описателями книг
#include "heterogeneous_lookup.hpp"  // прозрачные компараторы и хэш-функция для гетерогенного поиска
#include "statsistics.hpp"           // подключение функций расчета статистик картотеки книг

using namespace bookdb;

// Тест создания корректного описателя книги с заданием жанра с помощью строки
TEST(BookTest, BookWithStrGenre) {
    Book book("The Hobbit", "J.R.R. Tolkien", 1937, "Fiction", 4.9, 203);

    EXPECT_EQ(book.GetTitle(), "The Hobbit");
    EXPECT_EQ(book.GetAuthor(), "J.R.R. Tolkien");
    EXPECT_EQ(book.GetYear(), 1937);
    EXPECT_EQ(book.GetGenre(), Genre::Fiction);
    EXPECT_DOUBLE_EQ(book.GetRating(), 4.9);
    EXPECT_EQ(book.GetReadCount(), 203);
}

// Тест создания корректного описателя книги с заданием жанра с помощью перечисляемой константы
TEST(BookTest, BookWithEnumGenre) {
    Book book("Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98);

    EXPECT_EQ(book.GetTitle(), "Brave New World");
    EXPECT_EQ(book.GetAuthor(), "Aldous Huxley");
    EXPECT_EQ(book.GetYear(), 1932);
    EXPECT_EQ(book.GetGenre(), Genre::SciFi);
    EXPECT_DOUBLE_EQ(book.GetRating(), 4.5);
    EXPECT_EQ(book.GetReadCount(), 98);
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
        return LessByAuthor{}(a, b) && LessByAuthor{}(a, b.GetAuthor()) && LessByAuthor{}(a.GetAuthor(), b);
    };
    auto LessByTitleComp = [](const Book &a, const Book &b) {
        return LessByTitle{}(a, b) && LessByTitle{}(a, b.GetTitle()) && LessByTitle{}(a.GetTitle(), b);
    };
    auto LessByYearComp = [](const Book &a, const Book &b) {
        return LessByYear{}(a, b) && LessByYear{}(a, b.GetYear()) && LessByYear{}(a.GetYear(), b);
    };
    auto LessByRatingComp = [](const Book &a, const Book &b) {
        return LessByRating{}(a, b) && LessByRating{}(a, b.GetRating()) && LessByRating{}(a.GetRating(), b);
    };
    auto LessByPopularityComp = [](const Book &a, const Book &b) {
        return LessByPopularity{}(a, b) && LessByPopularity{}(a, b.GetReadCount()) &&
               LessByPopularity{}(a.GetReadCount(), b);
    };
    auto GreaterByYearComp = [](const Book &a, const Book &b) {
        return GreaterByYear{}(a, b) && GreaterByYear{}(a, b.GetYear()) && GreaterByYear{}(a.GetYear(), b);
    };
    auto GreaterByRatingComp = [](const Book &a, const Book &b) {
        return GreaterByRating{}(a, b) && GreaterByRating{}(a, b.GetRating()) && GreaterByRating{}(a.GetRating(), b);
    };
    auto GreaterByPopularityComp = [](const Book &a, const Book &b) {
        return GreaterByPopularity{}(a, b) && GreaterByPopularity{}(a, b.GetReadCount()) &&
               GreaterByPopularity{}(a.GetReadCount(), b);
    };

    // Создаем массив тестов для пар книг
    std::vector<TestCase> tests = {{"LessByAuthor", gatsby, hobbit, LessByAuthorComp, true},
                                   {"LessByTitle", gatsby, hobbit, LessByTitleComp, true},
                                   {"LessByYear", gatsby, hobbit, LessByYearComp, true},
                                   {"LessByRating", gatsby, hobbit, LessByRatingComp, true},
                                   {"LessByPopularity", gatsby, hobbit, LessByPopularityComp, true},
                                   {"GreaterByYear", world, gatsby, GreaterByYearComp, true},
                                   {"GreaterByRating", world, gatsby, GreaterByRatingComp, false},
                                   {"GreaterByPopularity", world, gatsby, GreaterByPopularityComp, false}};

    // Запускаем все тесты в цикле
    for (const auto &[name, lhs, rhs, comp, expected] : tests) {
        EXPECT_EQ(comp(lhs, rhs), expected) << "Failed: " << name;
    }
}

// Тест корректности работы концептов
TEST(BookTest, CorrectConcepts) {
    using namespace bookdb;
    using namespace bookdb::comp;

    // Контейнер картотеки книг должен отвечать концепту BookContainerLike
    static_assert(BookContainerLike<std::vector<Book>>);
    static_assert(!BookContainerLike<std::deque<Book>>);
    static_assert(!BookContainerLike<std::list<Book>>);
    static_assert(!BookContainerLike<std::array<Book, 10>>);

    // Итератор контейнера картотеки должен отвечать концепту BookIterator
    static_assert(BookIterator<std::vector<Book>::iterator>);
    static_assert(!BookIterator<std::deque<Book>::iterator>);
    static_assert(!BookIterator<std::list<Book>::iterator>);

    // Компараторы описателей книг должны отвечать концепту BookComparator
    static_assert(AllComparatorsValid<LessByAuthor, LessByTitle, LessByYear, LessByRating, LessByPopularity,
                                      GreaterByYear, GreaterByRating, GreaterByPopularity>);

    // Лямбда должна отвечать концепту BookPredicate
    auto isSciFi = [](const Book &b) { return b.GetGenre() == Genre::SciFi; };
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
        auto &empl_book = bookDbs[1].EmplaceBack(b.GetTitle(), b.GetAuthor(), b.GetYear(), b.GetGenre(), b.GetRating(),
                                                 b.GetReadCount());
        EXPECT_EQ(empl_book.GetTitle(), b.GetTitle());  // должна вернуться ссылка на созданную книгу
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
            EXPECT_TRUE(db.GetAuthors().contains(b.GetAuthor()));
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
        EXPECT_TRUE(db.GetAuthors().contains(b.GetAuthor()));
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
    EXPECT_EQ(it->GetTitle(), "The Hobbit");            // на 1-ом месте должен быть "The Hobbit"
    EXPECT_EQ((++it)->GetTitle(), "The Great Gatsby");  // на 2-ом месте - "The Great Gatsby"
    EXPECT_EQ((++it)->GetTitle(), "Brave New World");   // на 3-м месте - "Brave New World"
    EXPECT_EQ((++it)->GetTitle(), "Animal Farm");       // на 4-м месте - "Animal Farm"
    EXPECT_EQ((++it)->GetTitle(), "1984");              // на 5-м месте - "1984"

    // Проверяем поиск по условию
    auto hobbit_it = std::find_if(db.begin(), db.end(), [](const Book &b) { return b.GetYear() == 1937; });
    EXPECT_NE(hobbit_it, db.end());
}

// Тест функции построения гистограммы авторов
TEST(StatisticsTest, BuildAuthorHistogram) {
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
TEST(StatisticsTest, CalcGenreRatings) {
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

// Тест функции расчета среднего рейтинга всех книг в картотеке
TEST(StatisticsTest, CalcAvgRating) {
    using namespace bookdb;

    BookDatabase<std::vector<Book>> db;
    const size_t NUM_BOOKS = 10000;

    // Заполняем картотеку тестовыми книгами со случайными рейтингами
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> rating_dist(1.0, 5.0);

    double expected_avg = 0.0;
    for (size_t i = 0; i < NUM_BOOKS; ++i) {
        double rating = rating_dist(gen);
        expected_avg += rating;
        db.EmplaceBack("Book", "Author", 2026, Genre::Fiction, rating, 100);
    }

    expected_avg /= NUM_BOOKS;                       // ожидаемый средний рейтинг
    double tested_avg = calculateAverageRating(db);  // результат тестируемой функции

    // Результат функции должен совпадать с ожидаемым значением
    EXPECT_DOUBLE_EQ(tested_avg, expected_avg);
}

// Тест функции случайной выборки книг из картотеки
TEST(StatisticsTest, SampleRandomBooks) {
    using namespace bookdb;

    // Создаем тестовую картотеку из 10 книг
    BookDatabase db = {{"1984", "George Orwell", 1949, Genre::SciFi, 4.0, 190},
                       {"Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143},
                       {"The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120},
                       {"To Kill a Mockingbird", "Harper Lee", 1960, Genre::Fiction, 4.8, 156},
                       {"Pride and Prejudice", "Jane Austen", 1813, Genre::Fiction, 4.7, 178},
                       {"The Catcher in the Rye", "J.D. Salinger", 1951, Genre::Fiction, 4.3, 112},
                       {"Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98},
                       {"Jane Eyre", "Charlotte Brontë", 1847, Genre::Fiction, 4.6, 110},
                       {"The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203},
                       {"Lord of the Flies", "William Golding", 1954, Genre::Fiction, 4.2, 89}};

    // Тест 1: запрашиваем меньше книг, чем есть в картотеке
    auto sampling1 = sampleRandomBooks(db, 5);
    EXPECT_EQ(sampling1.size(), 5);  // должны получить 5 книг

    // Проверяем, что все элементы выборки - это книги из картотеки
    for (const auto &ref : sampling1) {
        const Book &book = ref.get();
        // Ищем каждую книгу в картотеке по названию и году
        auto it = std::find_if(db.begin(), db.end(), [&book](const Book &b) {
            return b.GetTitle() == book.GetTitle() && b.GetYear() == book.GetYear();
        });
        // Книга должна быть найдена в картотеке
        EXPECT_NE(it, db.end());
    }

    // Тест 2: Запрашиваем больше книг, чем есть в картотеке
    auto sampling2 = sampleRandomBooks(db, 100);
    EXPECT_EQ(sampling2.size(), db.size());  // должны получить все книги из картотеки

    // Тест 3: Запрашиваем 0 книг
    auto sampling3 = sampleRandomBooks(db, 0);
    EXPECT_TRUE(sampling3.empty());  // должны получить пустую выборку

    // Тест 4: Запрашиваем книги из пустой картотеки
    BookDatabase<std::vector<Book>> empty_db;
    auto sampling4 = sampleRandomBooks(empty_db, 3);
    EXPECT_TRUE(sampling4.empty());  // должны получить пустую выборку
}

// Тест функции для получения топ-N книг по заданному критерию
TEST(StatisticsTest, GetTopNBy) {
    using namespace bookdb;
    using namespace bookdb::comp;

    // Создаем тестовую картотеку книг из 5 книг с разными рейтингами
    BookDatabase db = {
        {"The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120},
        {"1984", "George Orwell", 1949, Genre::SciFi, 4.0, 190},
        {"Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98},
        {"Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143},
        {"The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203},
    };

    // Тест 1: Получим подборку топ-3 книг по убыванию рейтинга
    auto top_rating = getTopNBy(db, 3, GreaterByRating{});

    EXPECT_EQ(top_rating.size(), 3);  // в подборке должно быть 3 книги
    // В подборке должны быть книги с рейтингами 4.9, 4.5, 4.5
    EXPECT_EQ(top_rating[0].get().GetRating(), 4.9);  // "The Hobbit"
    EXPECT_EQ(top_rating[1].get().GetRating(), 4.5);  // The Great Gatsby" или "Brave New World"
    EXPECT_EQ(top_rating[2].get().GetRating(), 4.5);  // The Great Gatsby" или "Brave New World"

    // Тест 2: Получим подборку топ-5 книг по убыванию рейтинга (сортировка всех книг)
    auto top_all = getTopNBy(db, 5, GreaterByRating{});
    EXPECT_EQ(top_all.size(), 5);
    // В подборке должны быть книги с рейтингами 4.9 (1 место) и 4.0 (5 место)
    EXPECT_EQ(top_all[0].get().GetRating(), 4.9);  // "The Hobbit"
    EXPECT_EQ(top_all[4].get().GetRating(), 4.0);  // "1984"

    // Тест 3: Запрос топ-0 книг
    auto top_zero = getTopNBy(db, 0, GreaterByRating{});
    EXPECT_TRUE(top_zero.empty());

    // Тест 4: Запрос топ-3 книг из пустой картотеки
    BookDatabase<std::vector<Book>> empty_db;
    auto top_empty = getTopNBy(empty_db, 3, GreaterByRating{});
    EXPECT_TRUE(top_empty.empty());
}

// Тест фильтрации книг
TEST(FilterTest, FilterBooks) {
    using namespace bookdb;

    // Создаем тестовую картотеку книг из 5 книг
    BookDatabase db = {
        {"The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120},
        {"1984", "George Orwell", 1949, Genre::SciFi, 4.0, 190},
        {"Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98},
        {"Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143},
        {"The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203},
    };

    // Фильтр "И": научная фантастика с рейтингом >= 4.5 и год издания [1930, 1950]
    auto all_filter = all_of(GenreIs(Genre::SciFi), RatingAbove(4.5), YearBetween(1930, 1950));
    auto result_all = filterBooks(db.begin(), db.end(), all_filter);
    // Должна отобраться только одна книга - "Brave New World"
    EXPECT_EQ(result_all.size(), 1);
    EXPECT_EQ(result_all[0].get().GetTitle(), "Brave New World");

    // Фильтр "ИЛИ": или биография, или с рейтингом >= 4.9, или год в диапазоне [1995, 2000]
    auto any_filter = any_of(GenreIs(Genre::Biography), RatingAbove(4.9), YearBetween(1995, 2000));
    auto result_any = filterBooks(db.begin(), db.end(), any_filter);
    // Должна отобраться только одна книга - "The Hobbit"
    EXPECT_EQ(result_any.size(), 1);
    EXPECT_EQ(result_any[0].get().GetTitle(), "The Hobbit");

    // Фильтр "пустая выборка"
    auto none_filter = all_of(GenreIs(Genre::Biography), RatingAbove(5.0), YearBetween(2010, 2020));
    auto result_none = filterBooks(db.begin(), db.end(), none_filter);
    EXPECT_TRUE(result_none.empty());
}