#include "quick_sort.h"
#include <utility>

void quick_sort(int arr[], int left, int right) {
    int i = left;
    int j = right;
    int pivot = arr[(left+right)/2];
    while(i <= j) {
        while(arr[i] < pivot) i++;
        while(arr[j] > pivot) j--;
        if(i <= j) {
            std::swap(arr[i], arr[j]);
            i++;
            j--;
        }
    }
    if(left < j) quick_sort(arr, left, j);
    if(i < right) quick_sort(arr, i, right);
}