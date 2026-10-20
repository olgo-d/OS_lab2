
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>

#define REPEATS 5

void seq_mergeSort(int *arr, int len);
void par_mergeSort(int *arr, int len);
extern sem_t slots;

int sizes[] = {10000, 50000, 200000, 1000000};
int threads[] = {1, 2, 16, 100};

double getTime() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

double test(int *input, int n, int threadCount, int parallel) {
    int *arr = malloc(n * sizeof(int));
    double total = 0;

    if (arr == NULL) {
        perror("malloc");
        exit(1);
    }

    for (int i = 0; i < REPEATS; ++i) {
        memcpy(arr, input, n * sizeof(int));

        if (parallel)
            if (sem_init(&slots, 0, threadCount - 1) != 0){
                perror("sem_init");
                return 1;
            }

        double start = getTime();

        if (parallel)
            par_mergeSort(arr, n);
        else
            seq_mergeSort(arr, n);

        double end = getTime();

        if (parallel)
            if (sem_destroy(&slots) != 0) {
                perror("sem_destroy");
                return EXIT_FAILURE;
            }

        for (int j = 1; j < n; ++j) {
            if (arr[j - 1] > arr[j]) {
                printf("Sorting error!\n");
                exit(1);
            }
        }

        total += end - start;
    }

    free(arr);
    return total / REPEATS;
}

int main() {
    srand(42);

    for (int s = 0; s < 4; ++s) {
        int n = sizes[s];
        int *arr = malloc(n * sizeof(int));

        if (!arr) {
            perror("malloc");
            return 1;
        }

        for (int i = 0; i < n; ++i)
            arr[i] = rand();

        double seqTime = test(arr, n, 1, 0);

        printf("Array size: %d\n", n);
        printf("Sequential: %.6f s\n", seqTime);

        for (int t = 0; t < 4; ++t) {
            int count = threads[t];

            double parTime = test(arr, n, count, 1);
            double speedup = seqTime / parTime;
            double efficiency = speedup / count;

            printf("%d threads: %.6f s\n", count, parTime);
        }

        printf("\n");
        free(arr);
    }

    return 0;
}
