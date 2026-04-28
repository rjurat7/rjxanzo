#pragma once
#include <string>
#include <vector>

struct sqlite3;

class Database {
private:
    sqlite3* db;
    std::string dbPath;

public:
    // Конструкторы
    Database();
    Database(const std::string& path);
    ~Database();

    // Подключение к БД
    bool open();
    void close();
    bool isOpen() const;

    // Создание таблиц
    bool createTables();

    // Работа с пользователями
    bool addUser(const std::string& login, const std::string& password);
    bool checkUser(const std::string& login, const std::string& password);
    int getUserId(const std::string& login);

    // Работа с массивами
    bool saveArray(const std::string& login,
        const std::vector<int>& original,
        const std::vector<int>& sorted);

    std::vector<std::string> getUserArrays(const std::string& login);
    std::vector<std::vector<int>> getArraysForSorting(const std::string& login, int limit = 100);
    bool clearUserHistory(const std::string& login);

    // Тесты производительности
    double testPerformance(int count);
    double testLoadPerformance(int count);
    bool clearTestData();

    // Статистика
    int getArrayCount(const std::string& login);

    // Вспомогательные методы
    static std::string vectorToString(const std::vector<int>& vec);
    static std::vector<int> stringToVector(const std::string& str);
};