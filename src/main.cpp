#include <algorithm>  // подключение std::sort, std::for_each и др.
#include <print>  // подключение std::print

#include "book_database.hpp"  // заголовочный файл с классом картотеки книг и шаблоном ее форматирования
#include "comparators.hpp"    // компараторы для работы с описателями книг
#include "filters.hpp"        // фильтры для работы с описателями книг
#include "statsistics.hpp"    // подключение функций расчета статистик картотеки книг

using namespace bookdb;

int main() {
    //
    // Ниже приведён пример работы `BookDatabase`.
    //
    //     - Обратите внимание, что в этой функции реализованы основные возможности, охватывающие как обязательные, так
    //     и опциональные требования,
    //       которые не обязательны к реализации для сдачи работы.
    //     - Не забудьте перед созданием коммита вызвать 'run_clang_format.sh' для форматирования кода
    //

    // Создаем объект картотеки книг
    BookDatabase<std::vector<Book>> db;

    // Наполняем картотеку описателями книг
    db.EmplaceBack("1984", "George Orwell", 1949, Genre::SciFi, 4., 190);
    db.EmplaceBack("Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143);
    db.EmplaceBack("The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120);
    db.EmplaceBack("To Kill a Mockingbird", "Harper Lee", 1960, Genre::Fiction, 4.8, 156);
    db.EmplaceBack("Pride and Prejudice", "Jane Austen", 1813, Genre::Fiction, 4.7, 178);
    db.EmplaceBack("The Catcher in the Rye", "J.D. Salinger", 1951, Genre::Fiction, 4.3, 112);
    db.EmplaceBack("Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98);
    db.EmplaceBack("Jane Eyre", "Charlotte Brontë", 1847, Genre::Fiction, 4.6, 110);
    db.EmplaceBack("The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203);
    db.EmplaceBack("Lord of the Flies", "William Golding", 1954, Genre::Fiction, 4.2, 89);
    std::print("Books: {}\n\n", db);

    // Сортировка картотеки книг по авторам (по возрастанию)
    std::sort(db.begin(), db.end(), comp::LessByAuthor{});
    std::print("Books sorted by author: {}\n\n==================\n", db);

    // Сортировка картотеки книг по числу прочтений (по возрастанию)
    std::sort(db.begin(), db.end(), comp::LessByPopularity{});
    std::print("Books sorted by popularity (ascending): {}\n\n==================\n", db);

    // Сортировка картотеки книг по числу прочтений (по убыванию)
    std::sort(db.begin(), db.end(), comp::GreaterByPopularity{});
    std::print("Books sorted by popularity (descending ): {}\n\n==================\n", db);

    // Вывод гистограммы по авторам книг
    auto histogram = buildAuthorHistogramFlat(db);
    std::print("Author histogram:\n");
    std::for_each(histogram.begin(), histogram.end(),
                  [](const auto &pair) { std::print("  {}: {} books\n", pair.first, pair.second); });

    // Расчет средних рейтингов книг по жанрам
    auto genreRatings = calculateGenreRatings(db.begin(), db.end());
    std::print("\nAverage ratings by genres:\n");
    std::for_each(genreRatings.begin(), genreRatings.end(),
                  [](const auto &pair) { std::print("  {}: {:.2f}\n", pair.first, pair.second.first); });

    // Расчет среднего рейтинга книг по всей картотеке
    auto avrRating = calculateAverageRating(db);
    std::print("Average books rating in library: {}\n", avrRating);

    // Фильтрация всех книг 20-го века с рейтингом ≥ 4.5
    auto filtered = filterBooks(db.begin(), db.end(), all_of(YearBetween(1900, 1999), RatingAbove(4.5)));
    std::print("\n\nBooks from the 20th century with rating ≥ 4.5:\n");
    std::for_each(filtered.cbegin(), filtered.cend(), [](const auto &v) { std::print("{}\n", v.get()); });

    // Подборка топ-3 книг по рейтингу
    auto topBooks = getTopNBy(db, 3, comp::GreaterByRating{});
    std::print("\n\nTop 3 books by rating:\n");
    std::for_each(topBooks.cbegin(), topBooks.cend(), [](const auto &v) { std::print("{}\n", v.get()); });

    // Поиск книги Джорджа Оруэлла
    auto orwellBookIt =
        std::find_if(db.begin(), db.end(), [](const auto &v) { return v.GetAuthor() == "George Orwell"; });
    if (orwellBookIt != db.end()) {
        std::print("\n\nTransparent lookup by authors. Found Orwell's book: {}\n", *orwellBookIt);
    }

    return 0;
}