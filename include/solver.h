#ifndef SOLVER_H
#define SOLVER_H
#include "matrix_builder.h"
#include "membrane.h"

#include <mkl/mkl.h>

typedef struct {
    int n_eigenvalues;      // Nombre de valeurs à chercher
    double eps;            // Tolérance (pour d'éventuels solveurs itératifs)
    int mkl_threads;      // Nombre de threads MKL
} SolverConfig;

// Configuration du solveur
SolverConfig* create_solver_config(int n_eigenvalues);
void free_solver_config(SolverConfig* config);

// Résolution du problème (DSYGV uniquement)
EigenResults* solve_eigenproblem(SparseMatrixCSR* A, SparseMatrixCSR* B, 
                                 SolverConfig* config);

// Libération des résultats
void free_eigen_results(EigenResults* results);

// Utilitaires
void print_eigenvalues(EigenResults* results, int n_to_print);
void save_eigenresults(EigenResults* results, const char* filename);

#endif
