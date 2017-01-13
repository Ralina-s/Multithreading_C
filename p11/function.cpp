#include <stdio.h>
#include <stdlib.h>
#include <string> 
#include <math.h>
#include <time.h>
#include <openacc.h>
#include <iostream>

std::string welcome = "Функция для табулирования: 1 / exp(x) + cos(x) ^ 2 \n "
std::string input_data = "Ожидается ввод границ... \n";


#define PARTS_N 10000000
#define STEP_PRINT 1000000

int main() {

    std::cout << welcome;
    std::cout << input_data;

    double a, b;
    std::cin >> a >> b;

    double* y = (double *) calloc(PARTS_N + 2, sizeof(double));   // PARTS + a + b
    double delta = (b - a) / (PARTS_N + 1); 
    double time_begin = clock(), time_end; 

#pragma acc kernels
    for (int i = 0; i < PARTS_N + 2; i++) {
        y[i] = (1 / exp(a + delta * i) + cos(a + delta * i) * cos(a + delta * i));
    }

    time_end = clock();
    std::cout << "Time: " << (time_end - time_begin) / 1000.0 << std::endl;
    free(y);
    return 0;
}
