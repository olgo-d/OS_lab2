#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MIN_PARALLEL 10000

sem_t slots;

typedef struct {
    int *arr;
    int len;
} SortArgs;

typedef struct {
    int *arr;
    int *arr1;
    int *arr2;
    int left1, right1;
    int left2, right2;
    int outLeft;
} MergeArgs;

int tryStart(pthread_t *thread,
             void *(*func)(void *),
             void *args, int size) {

    if (size < MIN_PARALLEL || sem_trywait(&slots) != 0)
        return 0;

    if (pthread_create(thread, NULL, func, args) != 0) {
        if (sem_post(&slots) != 0) {
            perror("sem_post");
            exit(EXIT_FAILURE);
        }
        return 0;
    }
    return 1;
}

void waitAndRelease(pthread_t thread) {
    if (pthread_join(thread, NULL) != 0) {
        fprintf(stderr, "pthread_join failed\n");
        exit(EXIT_FAILURE);
    }
    if (sem_post(&slots) != 0) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }
}

int binSearch(int *arr, int left, int right, int key) {
    while (left < right) {
        int mid = left + (right - left) / 2;

        if (arr[mid] < key)
            left = mid + 1;
        else
            right = mid;
    }
    return left;
}
void merge(int *arr,
           int *arr1, int left1, int right1,
           int *arr2, int left2, int right2,
           int outLeft);
void mergeSort(int *arr, int len);

void *mergeThread(void *arg) {
    MergeArgs *a = (MergeArgs *)arg;

    merge(a->arr,
          a->arr1, a->left1, a->right1,
          a->arr2, a->left2, a->right2,
          a->outLeft);

    return NULL;
}

void merge(int *arr,
           int *arr1, int left1, int right1,
           int *arr2, int left2, int right2,
           int outLeft) {

    if (left1 > right1) {
        for (int i = left2; i <= right2; ++i)
            arr[outLeft++] = arr2[i];
        return;
    }

    if (left2 > right2) {
        for (int i = left1; i <= right1; ++i)
            arr[outLeft++] = arr1[i];
        return;
    }

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

    MergeArgs leftArgs = {
        arr, arr1, arr2,
        left1, mid - 1,
        left2, pos2 - 1,
        outLeft
    };

    pthread_t thread;

    int size = (right1 - left1 + 1)
             + (right2 - left2 + 1);

    int created = tryStart(
        &thread, mergeThread, &leftArgs, size
    );

    if (!created) {
        merge(arr,
              arr1, left1, mid - 1,
              arr2, left2, pos2 - 1,
              outLeft);
    }

    merge(arr,
          arr1, mid + 1, right1,
          arr2, pos2, right2,
          pos + 1);

    if (created)
        waitAndRelease(thread);
}

void *sortThread(void *arg) {
    SortArgs *a = (SortArgs *)arg;

    mergeSort(a->arr, a->len);
    return NULL;
}

void mergeSort(int *arr, int len) {
    if (len <= 1)
        return;

    int len1 = len / 2;
    int len2 = len - len1;

    int *arr1 = malloc((size_t)len1 * sizeof(int));
    int *arr2 = malloc((size_t)len2 * sizeof(int));

    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < len1; ++i)
        arr1[i] = arr[i];

    for (int i = 0; i < len2; ++i)
        arr2[i] = arr[len1 + i];

    SortArgs args = {arr1, len1};
    pthread_t thread;

    int created = tryStart(
        &thread, sortThread, &args, len
    );

    if (!created)
        mergeSort(arr1, len1);

    mergeSort(arr2, len2);

    if (created)
        waitAndRelease(thread);

    merge(arr, arr1, 0, len1 - 1,
          arr2, 0, len2 - 1, 0);

    free(arr1);
    free(arr2);
}

int main(int argc, char **argv){
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <threads>\n", argv[0]);
        return 1;
    }

    char *end;
    long threads = strtol(argv[1], &end, 10);

    if (*end != '\0' || threads < 1 || threads > 256) {
        fprintf(stderr, "Invalid thread count\n");
        return 1;
    }

    if (sem_init(&slots, 0, (unsigned)(threads - 1)) != 0) {
        perror("sem_init");
        return 1;
    }

    int arr[] = {8, 3, 5, 1, 9, 2, 7, 4, 6, 10};
    int len = sizeof(arr) / sizeof(arr[0]);

    mergeSort(arr, len);

    for (int i = 0; i < len; ++i) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    if (sem_destroy(&slots) != 0) {
        perror("sem_destroy");
        return EXIT_FAILURE;
    }

    return 0;
}