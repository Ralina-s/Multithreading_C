#!/bin/bash
mpic++ -o exp_mpi.mpi exp_mpi.cpp
mpirun -np 4 exp_mpi.mpi 55555
