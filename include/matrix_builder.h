#ifndef MATRIX_BUILDER_H
#define MATRIX_BUILDER_H
#include "mesh.h"

#include <mkl/mkl.h>


typedef struct {
    MKL_INT n_rows;
    MKL_INT n_cols;
    MKL_INT nnz;           // Nombre d'éléments non nuls
    double* values;       // Valeurs non nulles
    MKL_INT* columns;     // Indices de colonne
    MKL_INT* row_index;   // Indices de début de ligne
} SparseMatrixCSR;

// Construction des matrices
SparseMatrixCSR* build_stiffness_matrix(Mesh* mesh);
SparseMatrixCSR* build_mass_matrix(Mesh* mesh);

// Fonctions utilitaires pour matrices creuses
SparseMatrixCSR* create_sparse_matrix(MKL_INT n, MKL_INT nnz_estimate);
void free_sparse_matrix(SparseMatrixCSR* mat);
void save_matrix_csr(SparseMatrixCSR* mat, const char* filename);

// Conversion pour MKL
sparse_matrix_t convert_to_mkl_sparse(SparseMatrixCSR* csr);
void describe_matrix(SparseMatrixCSR* mat);

#endif
