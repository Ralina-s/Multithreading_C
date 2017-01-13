#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "omp.h"

#define MIN_N_FOR_PARALLEL 1000

void data_generate(double* array, double n) {
    for (int i = 0; i < n; i++) {
        array[i] = rand();
    }
}

void merge(double* array, int n) {
    int m = n / 2;
    int i = 0, j = m;

    double* buf = (double*) calloc(n, sizeof(double));
    int buf_cur = 0;

    while (i < m && j < n) {
        if (array[i] < array[j]) {
            buf[buf_cur] = array[i];
            buf_cur++;
            i++;
        } else {
            buf[buf_cur] = array[j];
            buf_cur++;
            j++;
        }
    }

    while (i < m) {
        buf[buf_cur] = array[i];
        buf_cur++;
        i++;
    }
    while (j < n) {
        buf[buf_cur] = array[j];
        buf_cur++;
        j++;
    }
    memcpy(array, buf, n * sizeof(double));
    free(buf);
}

void mergesort(double* array, int n) {

    if (n <= 1) {
        return;
    }

    int m = n / 2;

    // Для улучшения производительности при n < MIN_N_FOR_PARALLEL
    // перестаем делить на задачи
    if (n < MIN_N_FOR_PARALLEL) {
        mergesort(array, m);
        mergesort(array + m, n - m);
        merge(array, n);
    } else {
        #pragma omp task firstprivate (array, n)
        mergesort(array, m);

        #pragma omp task firstprivate (array, n)
        mergesort(array + m, n - m);

        #pragma omp taskwait
        merge(array, n);
    }

}
int main(int argc, char* argv[])
{
    std::cout << "______________________________" << std::endl;
    if (argc == 2) {
        std::cout << "Количество потоков: " << argv[1] << std::endl;
        omp_set_num_threads(atoi(argv[1]));
    } else {
        std::cout << "Количество потоков по умолчанию" << std::endl;
    }

    std::cout << std::endl <<  "Введите количество элементов в массиве: " << std::endl;
    int n;
    std::cin >> n;

    double time_begin, time_end;
    double* data = (double*) calloc(n, sizeof(double));

    data_generate(data, n);
    time_begin = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        mergesort(data, n);
    }
    time_end = omp_get_wtime();
    std::cout << std::endl << "Time:" << time_end - time_begin << std::endl;

    free(data);
}
