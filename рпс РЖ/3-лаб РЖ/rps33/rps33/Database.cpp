#include "Database.h"
#include "sqlite3.h"  
#include <sstream>
#include <iostream>
#include <chrono>
#include <random>
#include <algorithm>

// Конструкторы
Database::Database() : db(nullptr), dbPath("sorting.db") {}
Database::Database(const std::string& path) : db(nullptr), dbPath(path) {}

// Деструктор
Database::~Database() {
    close();
}

// Открытие БД
bool Database::open() {
    if (db != nullptr) {
        close();
    }

    int result = sqlite3_open(dbPath.c_str(), &db);
    if (result != SQLITE_OK) {
        std::cerr << "Ошибка открытия БД " << dbPath << ": " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Включаем проверку внешних ключей
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    return true;
}

// Закрытие БД
void Database::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

// Проверка, открыта ли БД
bool Database::isOpen() const {
    return db != nullptr;
}

// Создание таблиц
bool Database::createTables() {
    if (!isOpen()) return false;

    const char* usersSQL =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "login TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);";

    const char* arraysSQL =
        "CREATE TABLE IF NOT EXISTS arrays ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "original TEXT NOT NULL,"
        "sorted TEXT NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE);";

    char* errorMsg = nullptr;

    int result = sqlite3_exec(db, usersSQL, nullptr, nullptr, &errorMsg);
    if (result != SQLITE_OK) {
        if (errorMsg) {
            std::cerr << "SQL error (users): " << errorMsg << std::endl;
            sqlite3_free(errorMsg);
        }
        return false;
    }

    result = sqlite3_exec(db, arraysSQL, nullptr, nullptr, &errorMsg);
    if (result != SQLITE_OK) {
        if (errorMsg) {
            std::cerr << "SQL error (arrays): " << errorMsg << std::endl;
            sqlite3_free(errorMsg);
        }
        return false;
    }

    return true;
}

// Добавление пользователя
bool Database::addUser(const std::string& login, const std::string& password) {
    if (!isOpen()) return false;

    const char* sql = "INSERT OR IGNORE INTO users (login, password) VALUES (?, ?);";
    sqlite3_stmt* stmt;

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}

// Проверка пользователя
bool Database::checkUser(const std::string& login, const std::string& password) {
    if (!isOpen()) return false;

    const char* sql = "SELECT id FROM users WHERE login = ? AND password = ?;";
    sqlite3_stmt* stmt;

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    result = sqlite3_step(stmt);
    bool exists = (result == SQLITE_ROW);

    sqlite3_finalize(stmt);
    return exists;
}

// Получение id пользователя
int Database::getUserId(const std::string& login) {
    if (!isOpen()) return -1;

    const char* sql = "SELECT id FROM users WHERE login = ?;";
    sqlite3_stmt* stmt;

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);

    result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -1;
    }

    int userId = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    return userId;
}

// Преобразование вектора в строку
std::string Database::vectorToString(const std::vector<int>& vec) {
    std::stringstream ss;
    for (size_t i = 0; i < vec.size(); ++i) {
        ss << vec[i];
        if (i < vec.size() - 1) {
            ss << " ";
        }
    }
    return ss.str();
}

// Преобразование строки в вектор
std::vector<int> Database::stringToVector(const std::string& str) {
    std::vector<int> result;
    std::stringstream ss(str);
    int num;

    while (ss >> num) {
        result.push_back(num);
    }

    return result;
}

// Сохранение массивов
bool Database::saveArray(const std::string& login,
    const std::vector<int>& original,
    const std::vector<int>& sorted) {
    if (!isOpen()) return false;

    int userId = getUserId(login);
    if (userId == -1) {
        // Если пользователя нет, создаём его
        if (!addUser(login, "default_password")) {
            return false;
        }
        userId = getUserId(login);
        if (userId == -1) return false;
    }

    const char* sql = "INSERT INTO arrays (user_id, original, sorted) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return false;
    }

    std::string originalStr = vectorToString(original);
    std::string sortedStr = vectorToString(sorted);

    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_text(stmt, 2, originalStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, sortedStr.c_str(), -1, SQLITE_STATIC);

    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}

// Получение списка массивов пользователя
std::vector<std::string> Database::getUserArrays(const std::string& login) {
    std::vector<std::string> result;
    if (!isOpen()) return result;

    int userId = getUserId(login);
    if (userId == -1) return result;

    const char* sql = "SELECT original, sorted, created_at FROM arrays "
        "WHERE user_id = ? ORDER BY created_at DESC;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return result;
    }

    sqlite3_bind_int(stmt, 1, userId);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // Возврат: дата, оригинал, сортировка
        std::string row;

        const unsigned char* date = sqlite3_column_text(stmt, 2);
        const unsigned char* original = sqlite3_column_text(stmt, 0);
        const unsigned char* sorted = sqlite3_column_text(stmt, 1);

        if (date) row += reinterpret_cast<const char*>(date);
        row += "\t";
        if (original) row += reinterpret_cast<const char*>(original);
        row += "\t";
        if (sorted) row += reinterpret_cast<const char*>(sorted);

        result.push_back(row);
    }

    sqlite3_finalize(stmt);
    return result;
}

// Получение массивов для сортировки
std::vector<std::vector<int>> Database::getArraysForSorting(const std::string& login, int limit) {
    std::vector<std::vector<int>> result;
    if (!isOpen()) return result;

    int userId = getUserId(login);
    if (userId == -1) return result;

    const char* sql = "SELECT original FROM arrays "
        "WHERE user_id = ? ORDER BY RANDOM() LIMIT ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return result;
    }

    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, limit);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        if (text) {
            std::string str = reinterpret_cast<const char*>(text);
            result.push_back(stringToVector(str));
        }
    }

    sqlite3_finalize(stmt);
    return result;
}

// Очистка истории пользователя
bool Database::clearUserHistory(const std::string& login) {
    if (!isOpen()) return false;

    int userId = getUserId(login);
    if (userId == -1) return false;

    const char* sql = "DELETE FROM arrays WHERE user_id = ?;";
    sqlite3_stmt* stmt;

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, userId);
    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}

// Количество массивов пользователя
int Database::getArrayCount(const std::string& login) {
    if (!isOpen()) return 0;

    int userId = getUserId(login);
    if (userId == -1) return 0;

    const char* sql = "SELECT COUNT(*) FROM arrays WHERE user_id = ?;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, userId);
    rc = sqlite3_step(stmt);

    int count = 0;
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

// Тест производительности: вставка
double Database::testPerformance(int count) {
    if (!isOpen()) return 0.0;

    // Создать тестового пользователя
    addUser("test_user_perf", "test_pass");

    auto start = std::chrono::high_resolution_clock::now();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    for (int i = 0; i < count; i++) {
        int size = 5 + (i % 15); // 5–19 элементов
        std::vector<int> arr(size);
        for (int& num : arr) {
            num = dis(gen);
        }

        std::vector<int> sorted = arr;
        std::sort(sorted.begin(), sorted.end());

        if (!saveArray("test_user_perf", arr, sorted)) {
            return -1.0;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    return duration.count();
}

// Тест производительности: чтение
double Database::testLoadPerformance(int count) {
    if (!isOpen()) return 0.0;

    auto start = std::chrono::high_resolution_clock::now();

    auto arrays = getArraysForSorting("test_user_perf", count);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    return duration.count();
}

// Очистка тестовых данных
bool Database::clearTestData() {
    if (!isOpen()) return false;

    const char* sql = "DELETE FROM arrays WHERE user_id IN (SELECT id FROM users WHERE login LIKE 'test_user%');"
        "DELETE FROM users WHERE login LIKE 'test_user%';";

    char* errorMsg = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errorMsg);

    if (result != SQLITE_OK && errorMsg) {
        sqlite3_free(errorMsg);
        return false;
    }

    return true;
}