// shaker_sort.cpp
// Реализация шейкерной (коктейльной) сортировки

#include "shaker_sort.h"
#include <algorithm> // std::swap

// Шейкерная сортировка — улучшенная пузырьковая сортировка.
// Проходит по массиву попеременно слева направо и справа налево,
// сужая рабочий диапазон с каждым проходом.
//
// Временная сложность:
//   Лучший случай  — O(n):     массив уже отсортирован (0 обменов)
//   Средний случай — O(n^2)
//   Худший случай  — O(n^2):   массив отсортирован в обратном порядке
// Пространственная сложность: O(1) — сортировка «на месте»

void shakerSort(std::vector<int>& arr)
{
    int left  = 0;
    int right = static_cast<int>(arr.size()) - 1;
    bool swapped = true;

    while (left < right && swapped)
    {
        swapped = false;

        // Прямой проход: перемещаем максимум вправо
        for (int i = left; i < right; ++i)
        {
            if (arr[i] > arr[i + 1])
            {
                std::swap(arr[i], arr[i + 1]);
                swapped = true;
            }
        }
        --right; // правая граница сдвигается влево

        // Обратный проход: перемещаем минимум влево
        for (int i = right; i > left; --i)
        {
            if (arr[i] < arr[i - 1])
            {
                std::swap(arr[i], arr[i - 1]);
                swapped = true;
            }
        }
        ++left; // левая граница сдвигается вправо
    }
}
