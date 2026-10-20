#include <stdio.h>

int binSearch(int* arr, int left, int right, int key){
    while (left < right){
        int mid = left + (right - left) / 2;

        if (arr[mid] < key) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return left;
}

void merge(int* arr, int* arr1, int left1, int right1, int* arr2, int left2, int right2, int outLeft){
    if (left1 > right1) {
        for (int i = left2; i <= right2; ++i) {
            arr[outLeft++] = arr2[i];
        }
        return;
    }

    if (left2 > right2) {
        for (int i = left1; i <= right1; ++i) {
            arr[outLeft++] = arr1[i];
        }
        return;
    }

    // чтобы не дублтровать код, делаем условие, что 1-й участок короче
    if (right1 - left1 < right2 - left2) {
        merge(arr, arr2, left2, right2,
              arr1, left1, right1, outLeft);
        return;
    }

    int mid = left1 + (right1 - left1) / 2;
    int key = arr1[mid];

    int pos2 = binSearch(arr2, left2, right2 + 1, key);

    int pos = outLeft + (mid - left1) + (pos2 - left2);

    arr[pos] = key;

    merge(arr,
          arr1, left1, mid - 1,
          arr2, left2, pos2 - 1,
          outLeft);

    merge(arr,
          arr1, mid + 1, right1,
          arr2, pos2, right2,
          pos + 1);

    return;
}

void mergeSort(int* arr, int len){
    if (len <= 1){
        return;
    }

    int len1 = len/2;
    int len2 = len - len1;

    int arr1[len1], arr2[len2];

    for (int i = 0; i < len1; ++i){
        arr1[i] = arr[i];
    }
    for (int i = 0; i < len2; ++i){
        arr2[i] = arr[i + len1];
    }

    mergeSort(arr1, len1);
    mergeSort(arr2, len2);

    merge(arr, arr1, 0, len1 - 1, arr2, 0, len2 - 1, 0);

    return;
}

int main(){
    int arr1[4] = {1, 4, 6, 7};
    int arr2[5] = {2, 3, 5, 8, 9};
    int arr[9];
    merge(arr, arr1, 0, 3, arr2, 0, 4, 0);

    int arr3[10] = {10, 5, 4, 7, 6, 2, 9, 3, 8, 1};
    mergeSort(arr3, 10);

    for(int i = 0; i < 9; ++i){
        printf("%d ", arr[i]);
    }
    printf("\n");

    for(int i = 0; i < 9; ++i){
        printf("%d ", arr3[i]);
    }
    printf("\n");

    return 0;
}