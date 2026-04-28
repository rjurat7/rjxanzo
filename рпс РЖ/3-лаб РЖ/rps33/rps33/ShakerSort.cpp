#include "ShakerSort.h"
#include <algorithm>

void ShakerSort::sort(std::vector<int>& arr) {
    bool swapped = true;
    int left = 0;
    int right = (int)arr.size() - 1;

    while (swapped) {
        swapped = false;
        for (int i = left; i < right; ++i) {
            if (arr[i] > arr[i + 1]) {
                std::swap(arr[i], arr[i + 1]);
                swapped = true;
            }
        }
        right--;
        if (!swapped) break;

        swapped = false;
        for (int i = right; i > left; --i) {
            if (arr[i] < arr[i - 1]) {
                std::swap(arr[i], arr[i - 1]);
                swapped = true;
            }
        }
        left++;
    }
}