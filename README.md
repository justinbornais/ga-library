# C++ Library for the Implementation of Genetic Algorithms

This repository contains the necessary library functions for the implementation of genetic algorithms in C++20. This includes:
- Multiple template definitions for a min function which goes beyond std::min, as it allows for any number of variables to be used in the function;
- A Levenshtein function;
- A `char_mutator` class to facilitate character mutation;
- A mutate function; and
- A crossover function.

These functions also take full advantage of concepts in C++20. This is evident in the Levenshtein, mutate and crossover functions, which allow progarmmers to implement a genetic algorithm safely without incompatable types.
