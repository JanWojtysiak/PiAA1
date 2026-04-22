#include "merge_sort.h"
#include <vector>

void MergeSort::merge(int array[], int left, int mid, int right, bool ascending) {
    int left_size = mid - left + 1;
    int right_size = right - mid;

    std::vector<int> L(left_size);
    std::vector<int> R(right_size);

    for (int i = 0; i < left_size; i++) {
        L[i] = array[left + i];
    }

    for (int j = 0; j < right_size; j++) {
        R[j] = array[mid + 1 + j];
    }
    int i = 0;
    int j = 0;
    int k = left;


    while (i < left_size && j < right_size) {
        bool take_left = ascending ? (L[i] <= R[j]) : (L[i] >= R[j]);
        if (take_left) {
            array[k] = L[i];
            i++;
        } else {
            array[k] = R[j];
            j++;
        }
        k++;
    }
    
    while (i < left_size) {
        array[k] = L[i];
        i++;
        k++;
    }
    while (j < right_size) {
        array[k] = R[j];
        j++;
        k++;
    }

}
void MergeSort::merge_sort(int array[], int left, int right, bool ascending) {
    if (left >= right) return;
    int mid = (left + right) / 2;
    merge_sort(array, left, mid, ascending);
    merge_sort(array, mid + 1, right, ascending);
    merge(array, left, mid, right, ascending);

}