#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include "matrix_builder.h"
#include "membrane.h"
#include "mesh.h"

void save_mode_to_csv(Mesh* mesh, double* mode, int mode_index, 
                     const char* filename);

void generate_plots(Mesh* mesh, EigenResults* results, 
                   const char* output_dir);

void plot_convergence(int* grid_sizes, double** eigenvalues, 
                     int n_sizes, int n_eigenvalues,
                     const char* filename);

void plot_matrix_sparsity(SparseMatrixCSR* mat, const char* filename);

void create_animation(Mesh* mesh, EigenResults* results, 
                     int n_modes, const char* output_dir);

// Fonction pour générer les scripts Python
void generate_python_script(const char* plot_type);

#endif
