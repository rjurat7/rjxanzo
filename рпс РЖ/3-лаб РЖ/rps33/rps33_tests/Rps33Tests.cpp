#include <CppUnitTest.h>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "Database.h"
#include "ShakerSort.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace rps33_tests
{
    static std::string DbPath() {
        return (std::filesystem::temp_directory_path() / "rps33_vstest.db").string();
    }

    static void RemoveDbFile() {
        std::error_code ec;
        std::filesystem::remove(DbPath(), ec);
    }

    static std::wstring FmtTimeSec(double sec) {
        std::wstringstream ss;
        if (sec < 1.0) {
            ss << std::fixed << std::setprecision(2) << (sec * 1000.0) << L" мс";
        } else {
            ss << std::fixed << std::setprecision(3) << sec << L" с";
        }
        return ss.str();
    }

    static void LogResult(int num, const wchar_t* nameRu, double sec, bool success) {
        std::wstringstream w;
        w << L"Тест " << num << L". " << nameRu << L" | Время: " << FmtTimeSec(sec)
          << L" | Результат: " << (success ? L"успех" : L"ошибка") << L"\n";
        Logger::WriteMessage(w.str().c_str());
    }

    static double ElapsedSec(std::chrono::high_resolution_clock::time_point a,
        std::chrono::high_resolution_clock::time_point b) {
        return std::chrono::duration<double>(b - a).count();
    }

    TEST_CLASS(Rps33DbTests)
    {
    public:
        TEST_METHOD_INITIALIZE(Init)
        {
            RemoveDbFile();
        }

        TEST_METHOD_CLEANUP(Cleanup)
        {
            RemoveDbFile();
        }

        /* 1 */
        TEST_METHOD( Test01_OpenAndCreateTables )
        {
            auto t0 = std::chrono::high_resolution_clock::now();
            Database db(DbPath());
            bool ok = db.open() && db.createTables();
            db.close();
            auto t1 = std::chrono::high_resolution_clock::now();
            LogResult(1, L"Открытие БД и создание таблиц", ElapsedSec(t0, t1), ok);
            Assert::IsTrue(ok, L"open/createTables");
        }

        /* 2 */
        TEST_METHOD( Test02_AddUser )
        {
            auto t0 = std::chrono::high_resolution_clock::now();
            Database db(DbPath());
            bool ok = db.open() && db.createTables() && db.addUser("u1", "p1");
            db.close();
            auto t1 = std::chrono::high_resolution_clock::now();
            LogResult(2, L"Добавление пользователя", ElapsedSec(t0, t1), ok);
            Assert::IsTrue(ok, L"addUser");
        }

        /* 3 */
        TEST_METHOD( Test03_CheckUser )
        {
            auto t0 = std::chrono::high_resolution_clock::now();
            Database db(DbPath());
            bool ok = db.open() && db.createTables() && db.addUser("u2", "secret")
                && db.checkUser("u2", "secret");
            db.close();
            auto t1 = std::chrono::high_resolution_clock::now();
            LogResult(3, L"Проверка логина и пароля", ElapsedSec(t0, t1), ok);
            Assert::IsTrue(ok, L"checkUser");
        }

        /* 4 */
        TEST_METHOD( Test04_GetUserId )
        {
            auto t0 = std::chrono::high_resolution_clock::now();
            Database db(DbPath());
            bool ok = false;
            if (db.open() && db.createTables() && db.addUser("u3", "p")) {
                int id = db.getUserId("u3");
                ok = (id > 0);
            }
            db.close();
            auto t1 = std::chrono::high_resolution_clock::now();
            LogResult(4, L"Получение id пользователя", ElapsedSec(t0, t1), ok);
            Assert::IsTrue(ok, L"getUserId > 0");
        }

        /* 5 */
        TEST_METHOD( Test05_SaveArray )
        {
            auto t0 = std::chrono::high_resolution_clock::now();
            Database db(DbPath());
            bool ok = false;
            if (db.open() && db.createTables()) {
                std::vector<int> o = {2, 1, 3};
                std::vector<int> s = o;
                ShakerSort::sort(s);
                ok = db.saveArray("test_user", o, s);
            }
            db.close();
            auto t1 = std::chrono::high_resolution_clock::now();
            LogResult(5, L"Сохранение массива в БД", ElapsedSec(t0, t1), ok);
            Assert::IsTrue(ok, L"saveArray");
        }

        /* 6 */
        TEST_METHOD( Test06_GetUserArrays )
        {
            auto t0 = std::chrono::high_resolution_clock::now();
            Database db(DbPath());
            bool ok = false;
            if (db.open() && db.createTables()) {
                std::vector<int> o = {1};
                std::vector<int> s = o;
                if (db.saveArray("test_user", o, s)) {
                    auto vec = db.getUserArrays("test_user");
                    ok = !vec.empty();
                }
            }
            db.close();
            auto t1 = std::chrono::high_resolution_clock::now();
            LogResult(6, L"Список массивов пользователя", ElapsedSec(t0, t1), ok);
            Assert::IsTrue(ok, L"getUserArrays");
        }

        /* 7 */
        TEST_METHOD( Test07_GetArraysForSorting )
        {
            auto t0 = std::chrono::high_resolution_clock::now();
            Database db(DbPath());
            bool ok = false;
            if (db.open() && db.createTables()) {
                for (int k = 0; k < 3; ++k) {
                    std::vector<int> o = {k, k + 1};
                    std::vector<int> s = o;
                    ShakerSort::sort(s);
                    if (!db.saveArray("test_user", o, s)) {
                        o.clear();
                        break;
                    }
                }
                auto ar = db.getArraysForSorting("test_user", 2);
                ok = (ar.size() == 2);
            }
            db.close();
            auto t1 = std::chrono::high_resolution_clock::now();
            LogResult(7, L"Выборка массивов для сортировки", ElapsedSec(t0, t1), ok);
            Assert::IsTrue(ok, L"getArraysForSorting");
        }

        /* 8 */
        TEST_METHOD( Test08_ShakerSortOrder )
        {
            auto t0 = std::chrono::high_resolution_clock::now();
            std::vector<int> v = {4, 1, 3, 2};
            ShakerSort::sort(v);
            bool ok = (v == std::vector<int>({1, 2, 3, 4}));
            auto t1 = std::chrono::high_resolution_clock::now();
            LogResult(8, L"Сортировка Shaker (порядок элементов)", ElapsedSec(t0, t1), ok);
            Assert::IsTrue(ok, L"sort order");
        }

        /* 9 */
        TEST_METHOD( Test09_ClearUserHistory )
        {
            auto t0 = std::chrono::high_resolution_clock::now();
            Database db(DbPath());
            bool ok = false;
            if (db.open() && db.createTables()) {
                std::vector<int> o = {1, 0};
                std::vector<int> s = o;
                ShakerSort::sort(s);
                if (db.saveArray("test_user_9", o, s)) {
                    ok = db.clearUserHistory("test_user_9");
                    if (ok) {
                        ok = (db.getArrayCount("test_user_9") == 0);
                    }
                }
            }
            db.close();
            auto t1 = std::chrono::high_resolution_clock::now();
            LogResult(9, L"Очистка истории пользователя", ElapsedSec(t0, t1), ok);
            Assert::IsTrue(ok, L"clearUserHistory");
        }

        /* 10 */
        TEST_METHOD( Test10_ClearTestData )
        {
            auto t0 = std::chrono::high_resolution_clock::now();
            Database db(DbPath());
            bool ok = false;
            if (db.open() && db.createTables()) {
                std::vector<int> o = {1};
                std::vector<int> s = o;
                if (db.saveArray("test_user_z", o, s)) {
                    int before = db.getArrayCount("test_user_z");
                    if (db.clearTestData() && (before > 0)) {
                        int after = db.getArrayCount("test_user_z");
                        ok = (after == 0);
                    }
                }
            }
            db.close();
            auto t1 = std::chrono::high_resolution_clock::now();
            LogResult(10, L"Очистка тестовых данных (test_user% )", ElapsedSec(t0, t1), ok);
            Assert::IsTrue(ok, L"clearTestData");
        }
    };
}
