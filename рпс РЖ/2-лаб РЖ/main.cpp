// main.cpp
// Лабораторная работа №2 — Шейкерная сортировка
// Консольное приложение для сортировки целых чисел

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <limits>
#include <cstdlib>
#include <ctime>

#include "shaker_sort.h"

static int readInt(const std::string& prompt)
{
    while (true)
    {
        std::cout << prompt;
        int x;
        if (std::cin >> x)
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return x;
        }

        std::cout << "Ошибка ввода. Введите целое число.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

static int readNonNegativeInt(const std::string& prompt)
{
    while (true)
    {
        int x = readInt(prompt);
        if (x >= 0) return x;
        std::cout << "Число должно быть неотрицательным.\n";
    }
}

// ─────────────────────────────────────────────
// Вывод массива на экран
// ─────────────────────────────────────────────
void printArray(const std::vector<int>& arr)
{
    for (int i = 0; i < static_cast<int>(arr.size()); ++i)
    {
        std::cout << arr[i];
        if (i + 1 < static_cast<int>(arr.size()))
            std::cout << " ";
    }
    std::cout << "\n";
}

// ─────────────────────────────────────────────
// Ввод массива с клавиатуры
// ─────────────────────────────────────────────
std::vector<int> inputFromKeyboard()
{
    const int n = readNonNegativeInt("Введите количество элементов: ");

    std::vector<int> arr(n);
    std::cout << "Введите элементы массива:\n";
    for (int i = 0; i < n; ++i)
    {
        arr[i] = readInt("arr[" + std::to_string(i) + "] = ");
    }
    return arr;
}

// ─────────────────────────────────────────────
// Генерация случайного массива
// ─────────────────────────────────────────────
std::vector<int> generateArray()
{
    const int n = readNonNegativeInt("Введите количество элементов: ");

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::vector<int> arr(n);
    for (int i = 0; i < n; ++i)
        arr[i] = (std::rand() % 99) + 1; // числа от 1 до 99

    return arr;
}

// ─────────────────────────────────────────────
// Загрузка массива из файла
// Файл должен содержать числа, разделённые пробелами или переносами строк.
// Поддерживаются относительные и абсолютные пути (например: data\input.txt)
// ─────────────────────────────────────────────
std::vector<int> loadArrayFromFile()
{
    std::cout << "Введите путь к файлу (например: data\\input.txt или C:\\array.txt):\n> ";
    std::string filename;
    std::getline(std::cin, filename);

    // Убираем случайные пробелы в начале/конце
    while (!filename.empty() && (filename.front() == ' ' || filename.front() == '\t'))
        filename.erase(filename.begin());
    while (!filename.empty() && (filename.back() == ' ' || filename.back() == '\t' ||
           filename.back() == '\r' || filename.back() == '\n'))
        filename.pop_back();

    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cout << "Ошибка: не удалось открыть файл \"" << filename << "\"\n";
        std::cout << "Проверьте правильность пути и убедитесь, что файл существует.\n";
        return {};
    }

    std::vector<int> arr;
    int value;
    while (file >> value)
        arr.push_back(value);

    file.close();

    if (arr.empty())
        std::cout << "Предупреждение: файл пуст или не содержит чисел.\n";
    else
        std::cout << "Загружено элементов: " << arr.size() << "\n";

    return arr;
}

// ─────────────────────────────────────────────
// Сохранение исходного и отсортированного массивов в файл
// ─────────────────────────────────────────────
void saveToFile(const std::vector<int>& original, const std::vector<int>& sorted)
{
    std::cout << "Введите имя файла для сохранения (например: result.txt):\n> ";
    std::string filename;
    std::getline(std::cin, filename);

    // Убираем случайные пробелы
    while (!filename.empty() && (filename.back() == ' ' || filename.back() == '\t' ||
           filename.back() == '\r' || filename.back() == '\n'))
        filename.pop_back();

    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cout << "Ошибка: не удалось создать файл \"" << filename << "\"\n";
        std::cout << "Убедитесь, что папка существует и у вас есть права на запись.\n";
        return;
    }

    // Сохраняем исходный массив
    file << "Исходный массив:\n";
    for (int i = 0; i < static_cast<int>(original.size()); ++i)
    {
        file << original[i];
        if (i + 1 < static_cast<int>(original.size()))
            file << " ";
    }
    file << "\n\n";

    // Сохраняем отсортированный массив
    file << "Отсортированный массив:\n";
    for (int i = 0; i < static_cast<int>(sorted.size()); ++i)
    {
        file << sorted[i];
        if (i + 1 < static_cast<int>(sorted.size()))
            file << " ";
    }
    file << "\n";

    file.close();
    std::cout << "Файл успешно сохранён: " << filename << "\n";
}

// ─────────────────────────────────────────────
// Главное меню
// ─────────────────────────────────────────────
int main()
{
    int choice = 0;

    while (true)
    {
        std::cout << "\n=== МЕНЮ ===\n";
        std::cout << "1. Ввод с клавиатуры\n";
        std::cout << "2. Генерация случайных чисел\n";
        std::cout << "3. Загрузка из файла\n";
        std::cout << "4. Выход\n";
        choice = readInt("Ваш выбор: ");

        if (choice == 4)
        {
            std::cout << "Выход из программы.\n";
            break;
        }

        std::vector<int> arr;

        if (choice == 1)
            arr = inputFromKeyboard();
        else if (choice == 2)
            arr = generateArray();
        else if (choice == 3)
            arr = loadArrayFromFile();
        else
        {
            std::cout << "Неверный выбор. Попробуйте снова.\n";
            continue;
        }

        if (arr.empty())
        {
            std::cout << "Массив пуст. Возврат в меню.\n";
            continue;
        }

        // Сохраняем копию исходного массива
        std::vector<int> original = arr;

        std::cout << "\nИсходный массив:\n";
        printArray(original);

        // Сортировка шейкером
        shakerSort(arr);

        std::cout << "Отсортированный массив:\n";
        printArray(arr);

        // Предложить сохранить в файл
        const int save = readInt("\nСохранить оба массива в файл? (1 — да, 0 — нет): ");

        if (save == 1)
            saveToFile(original, arr);
    }

    return 0;
}
