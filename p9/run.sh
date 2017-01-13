#!/bin/bash
g++ ./merge_omp.cpp -fopenmp -o mergesort
./mergesort
./mergesort 1
