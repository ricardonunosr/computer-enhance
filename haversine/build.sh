#!/bin/bash

clang++ -g -o haversine_gen -Ilistings haversine_generator.cpp
clang++ -g -o haversine main.cpp

# ./haversine
./haversine_gen 1 234089 10000000