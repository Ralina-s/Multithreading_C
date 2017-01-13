##!/bin/bash
pgc++ -o func_without_acc function.cpp
pgc++ -acc -Minfo=accel -o func_with_acc function.cpp
./func_without_acc
./func_with_acc