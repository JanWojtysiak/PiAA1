#ifndef MERGE_SORT_H
#define MERGE_SORT_H

class MergeSort { 

   public:
    void merge(int arr[], int left, int mid, int right, bool ascending);    
    void merge_sort(int arr[], int left, int right, bool ascending = true);
};

#endif