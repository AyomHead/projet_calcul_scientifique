#include "matrix_builder.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAX_NNZ_PER_ROW 5  // Pour une grille 2D: diag + 4 voisins

SparseMatrixCSR* create_sparse_matrix(MKL_INT n, MKL_INT nnz_estimate) {
    SparseMatrixCSR* mat = (SparseMatrixCSR*)malloc(sizeof(SparseMatrixCSR));
    if (!mat) return NULL;
    
    mat->n_rows = n;
    mat->n_cols = n;
    mat->nnz = 0;
    
    // Allocation avec mkl_malloc pour l'alignement
    mat->values = (double*)mkl_malloc(nnz_estimate * sizeof(double), 64);
    mat->columns = (MKL_INT*)mkl_malloc(nnz_estimate * sizeof(MKL_INT), 64);
    mat->row_index = (MKL_INT*)mkl_malloc((n + 1) * sizeof(MKL_INT), 64);
    
    if (!mat->values || !mat->columns || !mat->row_index) {
        free_sparse_matrix(mat);
        return NULL;
    }
    
    mat->row_index[0] = 0;
    
    return mat;
}

void free_sparse_matrix(SparseMatrixCSR* mat) {
    if (!mat) return;
    
    if (mat->values) mkl_free(mat->values);
    if (mat->columns) mkl_free(mat->columns);
    if (mat->row_index) mkl_free(mat->row_index);
    free(mat);
}

SparseMatrixCSR* build_stiffness_matrix(Mesh* mesh) {
    int N = mesh->N;
    int total_points = mesh->total_points;
    double h = mesh->h;
    double h2 = h * h;
    
    // Estimation du nombre d'éléments non nuls
    MKL_INT nnz_estimate = total_points * MAX_NNZ_PER_ROW;
    SparseMatrixCSR* A = create_sparse_matrix(total_points, nnz_estimate);
    
    if (!A) return NULL;
    
    MKL_INT nnz = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int idx = mesh_index(i, j, mesh);
            A->row_index[idx] = nnz;
            
            // Coefficients diagonaux et voisins
            double diag_coeff = 0.0;
            
            // Contribution de p(i+1/2, j)
            if (i < N - 1) {
                int idx_right = mesh_index(i + 1, j, mesh);
                double p_half = 0.5 * (mesh->p_vals[idx] + mesh->p_vals[idx_right]);
                A->values[nnz] = -p_half / h2;
                A->columns[nnz] = idx_right;
                nnz++;
                diag_coeff += p_half / h2;
            }
            
            // Contribution de p(i-1/2, j)
            if (i > 0) {
                int idx_left = mesh_index(i - 1, j, mesh);
                double p_half = 0.5 * (mesh->p_vals[idx] + mesh->p_vals[idx_left]);
                A->values[nnz] = -p_half / h2;
                A->columns[nnz] = idx_left;
                nnz++;
                diag_coeff += p_half / h2;
            }
            
            // Contribution de p(i, j+1/2)
            if (j < N - 1) {
                int idx_up = mesh_index(i, j + 1, mesh);
                double p_half = 0.5 * (mesh->p_vals[idx] + mesh->p_vals[idx_up]);
                A->values[nnz] = -p_half / h2;
                A->columns[nnz] = idx_up;
                nnz++;
                diag_coeff += p_half / h2;
            }
            
            // Contribution de p(i, j-1/2)
            if (j > 0) {
                int idx_down = mesh_index(i, j - 1, mesh);
                double p_half = 0.5 * (mesh->p_vals[idx] + mesh->p_vals[idx_down]);
                A->values[nnz] = -p_half / h2;
                A->columns[nnz] = idx_down;
                nnz++;
                diag_coeff += p_half / h2;
            }
            
            // Terme diagonal final
            A->values[nnz] = diag_coeff + mesh->q_vals[idx];
            A->columns[nnz] = idx;
            nnz++;
        }
    }
    
    A->row_index[total_points] = nnz;
    A->nnz = nnz;
    
    return A;
}

SparseMatrixCSR* build_mass_matrix(Mesh* mesh) {
    int total_points = mesh->total_points;
    
    // La matrice de masse est diagonale
    SparseMatrixCSR* B = create_sparse_matrix(total_points, total_points);
    
    if (!B) return NULL;
    
    MKL_INT nnz = 0;
    
    for (int idx = 0; idx < total_points; idx++) {
        B->row_index[idx] = nnz;
        B->values[nnz] = mesh->w_vals[idx];
        B->columns[nnz] = idx;
        nnz++;
    }
    
    B->row_index[total_points] = nnz;
    B->nnz = nnz;
    
    return B;
}

void save_matrix_csr(SparseMatrixCSR* mat, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return;
    
    fprintf(file, "row,col,value\n");
    for (MKL_INT i = 0; i < mat->n_rows; i++) {
        for (MKL_INT j = mat->row_index[i]; j < mat->row_index[i + 1]; j++) {
            fprintf(file, "%d,%d,%.6e\n", i, mat->columns[j], mat->values[j]);
        }
    }
    
    fclose(file);
}

sparse_matrix_t convert_to_mkl_sparse(SparseMatrixCSR* csr) {
    sparse_matrix_t mkl_mat;
    sparse_status_t status;
    
    status = mkl_sparse_d_create_csr(
        &mkl_mat,
        SPARSE_INDEX_BASE_ZERO,
        csr->n_rows,
        csr->n_cols,
        csr->row_index,
        csr->row_index + 1,
        csr->columns,
        csr->values
    );
    
    if (status != SPARSE_STATUS_SUCCESS) {
        printf("Error converting to MKL sparse matrix: %d\n", status);
    }
    
    return mkl_mat;
}

void describe_matrix(SparseMatrixCSR* mat) {
    printf("Matrix: %d x %d\n", mat->n_rows, mat->n_cols);
    printf("Non-zero elements: %d\n", mat->nnz);
    printf("Sparsity: %.4f%%\n", 
           100.0 * (1.0 - (double)mat->nnz / (mat->n_rows * mat->n_cols)));
}
