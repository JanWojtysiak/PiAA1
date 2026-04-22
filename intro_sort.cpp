#include "intro_sort.h"
#include <utility>

// Próg, poniżej którego używamy sortowania przez wstawianie.
// Dla małych tablic jest ono szybsze niż quicksort/heapsort.
static const int INSERTION_THRESHOLD = 16;

// Liczy floor(log2(n)) bez użycia funkcji zmiennoprzecinkowych.
// Dzięki temu unikamy problemów z zaokrągleniami.
static int floor_log2(int n) {
    int result = 0;
    while (n > 1) {
        n /= 2;
        result++;
    }
    return result;
}

// --- Sortowanie przez wstawianie ---
static void insertion_sort(int arr[], int left, int right, bool ascending) {
    for (int i = left + 1; i <= right; i++) {
        int key = arr[i];
        int j = i - 1;
        // Przesuwamy w prawo elementy większe od key.
        while (j >= left && (ascending ? (arr[j] > key) : (arr[j] < key))) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

static void heapify(int arr[], int n, int i, int offset, bool ascending) {
    while (true) {
        int largest = i;
        int l = 2 * i + 1;
        int r = 2 * i + 2;

        if (l < n) {
            bool left_better = ascending
                ? (arr[offset + l] > arr[offset + largest])
                : (arr[offset + l] < arr[offset + largest]);
            if (left_better) largest = l;
        }

        if (r < n) {
            bool right_better = ascending
                ? (arr[offset + r] > arr[offset + largest])
                : (arr[offset + r] < arr[offset + largest]);
            if (right_better) largest = r;
        }

        if (largest == i) return;

        std::swap(arr[offset + i], arr[offset + largest]);
        i = largest;
    }
}

// --- Heapsort ---
// Gwarantuje złożoność O(n log n) niezależnie od danych.
static void heap_sort(int arr[], int left, int right, bool ascending) {
    int n = right - left + 1;
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i, left, ascending);
    }
    for (int i = n - 1; i > 0; i--) {
        std::swap(arr[left], arr[left + i]);
        heapify(arr, i, 0, left, ascending);
    }
}

// ---  introsort ---

static void intro_sort_util(int arr[], int left, int right, int depth_limit, bool ascending) {
    while (right - left + 1 >= INSERTION_THRESHOLD) {
        // Za głęboka rekurencja -> przełączamy się na heapsort.
        if (depth_limit == 0) {
            heap_sort(arr, left, right, ascending);
            return;
        }
        depth_limit--;
        int i = left;
        int j = right;
        int pivot = arr[(left + right) / 2];
        while (i <= j) {
            if (ascending) {
                while (arr[i] < pivot) i++;
                while (arr[j] > pivot) j--;
            } else {
                while (arr[i] > pivot) i++;
                while (arr[j] < pivot) j--;
            }
            if (i <= j) {
                std::swap(arr[i], arr[j]);
                i++;
                j--;
            }
        }

        if (j - left < right - i) {
            if (left < j) intro_sort_util(arr, left, j, depth_limit, ascending);
            left = i;
        } else {
            if (i < right) intro_sort_util(arr, i, right, depth_limit, ascending);
            right = j;
        }
    }
    if (left < right) insertion_sort(arr, left, right, ascending);
}

void intro_sort(int arr[], int left, int right, bool ascending) {
    if (arr == nullptr || left >= right) return;

    int n = right - left + 1;
    int depth_limit = 2 * floor_log2(n);

    intro_sort_util(arr, left, right, depth_limit, ascending);
}
