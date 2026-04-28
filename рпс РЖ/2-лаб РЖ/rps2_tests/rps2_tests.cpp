#include "pch.h"
#include "CppUnitTest.h"
#include <vector>
#include "../shaker_sort.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace rps2tests
{
    TEST_CLASS(ShakerSortTest)
    {
    public:

        // Тест 1: Обычный массив
        TEST_METHOD(Test_NormalArray)
        {
            std::vector<int> arr = { 5, 3, 8, 1, 9, 2 };
            std::vector<int> expected = { 1, 2, 3, 5, 8, 9 };
            shakerSort(arr);
            Assert::IsTrue(arr == expected);
        }

        // Тест 2: Уже отсортированный массив
        TEST_METHOD(Test_AlreadySorted)
        {
            std::vector<int> arr = { 1, 2, 3, 4, 5 };
            std::vector<int> expected = { 1, 2, 3, 4, 5 };
            shakerSort(arr);
            Assert::IsTrue(arr == expected);
        }

        // Тест 3: Обратно отсортированный массив
        TEST_METHOD(Test_ReverseSorted)
        {
            std::vector<int> arr = { 9, 7, 5, 3, 1 };
            std::vector<int> expected = { 1, 3, 5, 7, 9 };
            shakerSort(arr);
            Assert::IsTrue(arr == expected);
        }

        // Тест 4: Один элемент
        TEST_METHOD(Test_SingleElement)
        {
            std::vector<int> arr = { 42 };
            std::vector<int> expected = { 42 };
            shakerSort(arr);
            Assert::IsTrue(arr == expected);
        }

        // Тест 5: Пустой массив
        TEST_METHOD(Test_EmptyArray)
        {
            std::vector<int> arr;
            shakerSort(arr);
            Assert::IsTrue(arr.empty());
        }

        // Тест 6: Отрицательные числа
        TEST_METHOD(Test_NegativeNumbers)
        {
            std::vector<int> arr = { -3, -1, -7, -2, -5 };
            std::vector<int> expected = { -7, -5, -3, -2, -1 };
            shakerSort(arr);
            Assert::IsTrue(arr == expected);
        }

        // Тест 7: Одинаковые элементы
        TEST_METHOD(Test_AllEqual)
        {
            std::vector<int> arr = { 4, 4, 4, 4, 4 };
            std::vector<int> expected = { 4, 4, 4, 4, 4 };
            shakerSort(arr);
            Assert::IsTrue(arr == expected);
        }

        // Тест 8: Два элемента
        TEST_METHOD(Test_TwoElements)
        {
            std::vector<int> arr = { 10, 2 };
            std::vector<int> expected = { 2, 10 };
            shakerSort(arr);
            Assert::IsTrue(arr == expected);
        }

        // Тест 9: Смесь отрицательных и положительных
        TEST_METHOD(Test_MixedNumbers)
        {
            std::vector<int> arr = { 3, -5, 0, 8, -1, 2 };
            std::vector<int> expected = { -5, -1, 0, 2, 3, 8 };
            shakerSort(arr);
            Assert::IsTrue(arr == expected);
        }

        // Тест 10: Большой массив
        TEST_METHOD(Test_LargeArray)
        {
            std::vector<int> arr(100);
            for (int i = 0; i < 100; ++i)
                arr[i] = 100 - i;

            std::vector<int> expected(100);
            for (int i = 0; i < 100; ++i)
                expected[i] = i + 1;

            shakerSort(arr);
            Assert::IsTrue(arr == expected);
        }
    };
}