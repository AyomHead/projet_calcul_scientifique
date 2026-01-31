#include "solver.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Définitions pour PI si non défini
#ifndef PI
#define PI 3.14159265358979323846
#endif

SolverConfig* create_solver_config(int n_eigenvalues) {
    SolverConfig* config = (SolverConfig*)malloc(sizeof(SolverConfig));
    if (!config) {
        fprintf(stderr, "Error: Failed to allocate solver config\n");
        return NULL;
    }
    
    config->n_eigenvalues = n_eigenvalues;
    config->eps = 1e-10;
    config->mkl_threads = 4;
    
    return config;
}

void free_solver_config(SolverConfig* config) {
    if (config) free(config);
}

EigenResults* solve_eigenproblem(SparseMatrixCSR* A, SparseMatrixCSR* B, 
                                 SolverConfig* config) {
    printf("\n=== SOLVING EIGENPROBLEM (DSYGV DENSE SOLVER) ===\n");
    
    clock_t start = clock();
    int n = (int)A->n_rows;
    int k = config->n_eigenvalues;
    
    if (k > n) {
        printf("Warning: Requested %d eigenvalues but only %d DOF. Using %d instead.\n", 
               k, n, n);
        k = n;
    }
    
    printf("Problem size: %d x %d\n", n, n);
    printf("Requested eigenvalues: %d\n", k);
    
    // Allouer résultats
    EigenResults* results = (EigenResults*)malloc(sizeof(EigenResults));
    if (!results) {
        fprintf(stderr, "Error: Failed to allocate eigen results\n");
        return NULL;
    }
    
    results->n_eigenvalues = k;
    results->eigenvalues = (double*)malloc(k * sizeof(double));
    results->residuals = (double*)malloc(k * sizeof(double));
    results->eigenvectors = (double**)malloc(k * sizeof(double*));
    
    if (!results->eigenvalues || !results->residuals || !results->eigenvectors) {
        fprintf(stderr, "Error: Failed to allocate eigen results arrays\n");
        free_eigen_results(results);
        return NULL;
    }
    
    for (int i = 0; i < k; i++) {
        results->eigenvectors[i] = (double*)malloc(n * sizeof(double));
        if (!results->eigenvectors[i]) {
            fprintf(stderr, "Error: Failed to allocate eigenvector %d\n", i);
            free_eigen_results(results);
            return NULL;
        }
        results->residuals[i] = 0.0;  // DSYGV ne calcule pas de résidu
    }
    
    // ===== CONVERSION CSR -> DENSE (symétrique) =====
    printf("Converting CSR matrices to dense format...\n");
    
    double* A_dense = (double*)calloc(n * n, sizeof(double));
    double* B_dense = (double*)calloc(n * n, sizeof(double));
    
    if (!A_dense || !B_dense) {
        fprintf(stderr, "Error: Failed to allocate dense matrices\n");
        free(A_dense);
        free(B_dense);
        free_eigen_results(results);
        return NULL;
    }
    
    // Convertir A (CSR -> dense, symétrique)
    for (int i = 0; i < n; i++) {
        for (MKL_INT j = A->row_index[i]; j < A->row_index[i+1]; j++) {
            int col = (int)A->columns[j];
            A_dense[i * n + col] = A->values[j];
            // Pour symétrie, copier aussi l'élément symétrique
            if (col != i) {
                A_dense[col * n + i] = A->values[j];
            }
        }
    }
    
    // Convertir B (diagonale -> dense, symétrique)
    // NOTE: B est diagonal, mais on le convertit en dense pour DSYGV
    for (int i = 0; i < n; i++) {
        for (MKL_INT j = B->row_index[i]; j < B->row_index[i+1]; j++) {
            int col = (int)B->columns[j];
            B_dense[i * n + col] = B->values[j];
            if (col != i) {
                B_dense[col * n + i] = B->values[j];
            }
        }
    }
    
    // ===== RÉSOLUTION AVEC DSYGV =====
    printf("Calling DSYGV (dense symmetric generalized eigenproblem)...\n");
    
    char jobz = 'V';      // Calculer valeurs ET vecteurs propres
    char uplo = 'U';      // Utiliser triangle supérieur
    MKL_INT itype = 1;    // A*x = lambda*B*x
    MKL_INT lda = n;
    MKL_INT ldb = n;
    MKL_INT info;
    
    // Tableau complet des valeurs propres (n éléments)
    double* all_eigenvalues = (double*)malloc(n * sizeof(double));
    if (!all_eigenvalues) {
        fprintf(stderr, "Error: Failed to allocate eigenvalues array\n");
        free(A_dense);
        free(B_dense);
        free_eigen_results(results);
        return NULL;
    }
    
    // Query pour la taille optimale du workspace
    MKL_INT lwork = -1;
    double work_query;
    
    dsygv(&itype, &jobz, &uplo, &n, A_dense, &lda, B_dense, &ldb, 
          all_eigenvalues, &work_query, &lwork, &info);
    
    if (info != 0) {
        printf("Warning: Workspace query returned info = %ld\n", (long)info);
        // Continuer avec une taille par défaut
        lwork = 3 * n;
    } else {
        lwork = (MKL_INT)work_query;
    }
    
    double* work = (double*)malloc(lwork * sizeof(double));
    if (!work) {
        fprintf(stderr, "Error: Failed to allocate workspace\n");
        free(A_dense);
        free(B_dense);
        free(all_eigenvalues);
        free_eigen_results(results);
        return NULL;
    }
    
    // Résoudre le problème complet
    dsygv(&itype, &jobz, &uplo, &n, A_dense, &lda, B_dense, &ldb,
          all_eigenvalues, work, &lwork, &info);
    
    printf("DSYGV completed with info = %ld\n", (long)info);
    
    if (info != 0) {
        printf("Warning: DSYGV failed with error code %ld\n", (long)info);
        results->n_eigenvalues = 0;
    } else {
        // DSYGV trie les valeurs propres dans l'ordre croissant
        // On prend les k plus petites (les premiers modes)
        for (int i = 0; i < k; i++) {
            results->eigenvalues[i] = all_eigenvalues[i];
            
            // Les vecteurs propres sont stockés dans les colonnes de A_dense
            for (int j = 0; j < n; j++) {
                results->eigenvectors[i][j] = A_dense[j * n + i];
            }
            
            // Normaliser le vecteur propre (optionnel mais utile)
            double norm = 0.0;
            for (int j = 0; j < n; j++) {
                norm += results->eigenvectors[i][j] * results->eigenvectors[i][j];
            }
            norm = sqrt(norm);
            if (norm > 1e-12) {
                for (int j = 0; j < n; j++) {
                    results->eigenvectors[i][j] /= norm;
                }
            }
        }
        
        printf("\nSuccessfully computed %d eigenvalues:\n", k);
    }
    
    // ===== NETTOYAGE =====
    free(A_dense);
    free(B_dense);
    free(all_eigenvalues);
    free(work);
    
    clock_t end = clock();
    results->computation_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    results->iterations = 1;  // DSYGV est direct
    
    printf("Computation time: %.3f seconds\n", results->computation_time);
    
    return results;
}

void free_eigen_results(EigenResults* results) {
    if (!results) return;
    
    if (results->eigenvalues) {
        free(results->eigenvalues);
        results->eigenvalues = NULL;
    }
    
    if (results->residuals) {
        free(results->residuals);
        results->residuals = NULL;
    }
    
    if (results->eigenvectors) {
        for (int i = 0; i < results->n_eigenvalues; i++) {
            if (results->eigenvectors[i]) {
                free(results->eigenvectors[i]);
                results->eigenvectors[i] = NULL;
            }
        }
        free(results->eigenvectors);
        results->eigenvectors = NULL;
    }
    
    free(results);
}

void print_eigenvalues(EigenResults* results, int n_to_print) {
    if (!results || !results->eigenvalues) {
        printf("No eigenvalues to print\n");
        return;
    }
    
    int n = (n_to_print < results->n_eigenvalues) ? n_to_print : results->n_eigenvalues;
    
    printf("\n=== EIGENVALUES ===\n");
    printf("Index    Eigenvalue    Frequency (Hz)\n");
    printf("-------------------------------------\n");
    
    for (int i = 0; i < n; i++) {
        double freq = sqrt(results->eigenvalues[i]) / (2 * PI);
        printf("%3d    %12.6f    %8.3f\n", i + 1, results->eigenvalues[i], freq);
    }
}

void save_eigenresults(EigenResults* results, const char* filename) {
    if (!results || !filename) return;
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    
    fprintf(file, "index,eigenvalue,frequency_hz\n");
    for (int i = 0; i < results->n_eigenvalues; i++) {
        double freq = sqrt(results->eigenvalues[i]) / (2 * PI);
        fprintf(file, "%d,%.10e,%.10e\n", i + 1, results->eigenvalues[i], freq);
    }
    
    fclose(file);
    printf("Saved eigenvalues to %s\n", filename);
}
