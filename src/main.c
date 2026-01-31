#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "membrane.h"
#include "mesh.h"
#include "matrix_builder.h"
#include "solver.h"
#include "visualization.h"

// Définitions pour PI si non défini
#ifndef PI
#define PI 3.14159265358979323846
#endif

int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("  Membrane Vibration Solver\n");
    printf("  Using Intel MKL\n");
    printf("========================================\n\n");
    
    clock_t start_time = clock();
    
    // ============ CONFIGURATION ============
    int N = 50;                     // Points par dimension (50x50 = 2500 DOF)
    int n_eigenvalues = 10;        // Nombre de modes à calculer
    
    if (argc > 1) N = atoi(argv[1]);
    if (argc > 2) n_eigenvalues = atoi(argv[2]);
    
    // Validation des paramètres
    if (N < 10) {
        fprintf(stderr, "Error: Grid size N must be at least 10\n");
        return 1;
    }
    if (n_eigenvalues < 1) {
        fprintf(stderr, "Error: Must compute at least 1 eigenvalue\n");
        return 1;
    }
    if (n_eigenvalues > N * N) {
        fprintf(stderr, "Error: Cannot compute more eigenvalues than DOF\n");
        return 1;
    }
    
    printf("Configuration:\n");
    printf("  Grid size: %d x %d\n", N, N);
    printf("  Total DOF: %d\n", N * N);
    printf("  Eigenvalues to compute: %d\n\n", n_eigenvalues);
    
    // ============ INITIALISATION MKL ============
    int mkl_threads = 4;
    mkl_set_num_threads(mkl_threads);
    printf("MKL configured with %d threads\n\n", mkl_threads);
    
    // ============ CREATION DU PROBLEME ============
    printf("Creating membrane problem...\n");
    MembraneParams* params = create_default_params();
    if (!params) {
        fprintf(stderr, "Error: Failed to create membrane parameters\n");
        return 1;
    }
    
    Mesh* mesh = create_mesh(N, params);
    if (!mesh) {
        fprintf(stderr, "Error: Failed to create mesh\n");
        free_membrane_params(params);
        return 1;
    }
    
    printf("Mesh created with h = %.6f\n", mesh->h);
    printf("Saving mesh data...\n");
    
    // Créer le répertoire data s'il n'existe pas
    int ret = system("mkdir -p data"); (void)ret;
    
    save_mesh(mesh, "data/mesh_data.csv");
    
    // ============ CONSTRUCTION DES MATRICES ============
    printf("\nBuilding stiffness matrix A...\n");
    SparseMatrixCSR* A = build_stiffness_matrix(mesh);
    if (!A) {
        fprintf(stderr, "Error: Failed to build stiffness matrix\n");
        free_mesh(mesh);
        free_membrane_params(params);
        return 1;
    }
    
    printf("A: %d x %d, NNZ = %d (Sparsity: %.2f%%)\n", 
           (int)A->n_rows, (int)A->n_cols, (int)A->nnz, 
           100.0 * A->nnz / ((double)A->n_rows * A->n_cols));
    
    printf("Building mass matrix B...\n");
    SparseMatrixCSR* B = build_mass_matrix(mesh);
    if (!B) {
        fprintf(stderr, "Error: Failed to build mass matrix\n");
        free_sparse_matrix(A);
        free_mesh(mesh);
        free_membrane_params(params);
        return 1;
    }
    
    printf("B: %d x %d, NNZ = %d\n", 
           (int)B->n_rows, (int)B->n_cols, (int)B->nnz);
    
    // Sauvegarde des matrices pour analyse
    save_matrix_csr(A, "data/matrix_A_pattern.csv");
    save_matrix_csr(B, "data/matrix_B_pattern.csv");
    plot_matrix_sparsity(A, "plots/matrix_sparsity.png");
    
    // ============ CONFIGURATION DU SOLVEUR ============
    printf("\nConfiguring solver...\n");
    SolverConfig* config = create_solver_config(n_eigenvalues);
    if (!config) {
        fprintf(stderr, "Error: Failed to create solver configuration\n");
        free_sparse_matrix(A);
        free_sparse_matrix(B);
        free_mesh(mesh);
        free_membrane_params(params);
        return 1;
    }
    
    // ============ RESOLUTION ============
    printf("\nSolving eigenvalue problem...\n");
    clock_t solve_start = clock();
    
    EigenResults* results = solve_eigenproblem(A, B, config);
    
    clock_t solve_end = clock();
    double solve_time = ((double)(solve_end - solve_start)) / CLOCKS_PER_SEC;
    
    if (!results) {
        fprintf(stderr, "Error: Eigenvalue solver failed\n");
        free_solver_config(config);
        free_sparse_matrix(A);
        free_sparse_matrix(B);
        free_mesh(mesh);
        free_membrane_params(params);
        return 1;
    }
    
    printf("Solution completed in %.2f seconds\n", solve_time);
    printf("Convergence: %d iterations\n", results->iterations);
    printf("Residuals: ");
    for (int i = 0; i < 3 && i < results->n_eigenvalues; i++) {
        printf("%.2e ", results->residuals[i]);
    }
    printf("\n");
    
    // ============ RESULTATS ============
    printf("\n=== EIGENVALUES ===\n");
    print_eigenvalues(results, n_eigenvalues);
    
    // Fréquences correspondantes (en Hz si on suppose certaines unités)
    printf("\n=== FREQUENCIES (Hz) ===\n");
    for (int i = 0; i < results->n_eigenvalues; i++) {
        double freq = sqrt(results->eigenvalues[i]) / (2 * PI);
        printf("Mode %2d: λ = %10.6f, f = %8.3f Hz\n", 
               i+1, results->eigenvalues[i], freq);
    }
    
    // ============ VISUALISATION ============
    printf("\nGenerating visualizations...\n");
    
    // Créer le répertoire data s'il n'existe pas
    ret = system("mkdir -p data"); (void)ret;
    
    // Sauvegarde des modes propres avec messages DEBUG
    int modes_to_save = (results->n_eigenvalues < 5) ? results->n_eigenvalues : 5;
    for (int i = 0; i < modes_to_save; i++) {
        char filename[256];
        sprintf(filename, "data/mode_%02d.csv", i+1);
        printf("DEBUG: Saving mode %d to %s\n", i+1, filename);
        save_mode_to_csv(mesh, results->eigenvectors[i], i, filename);
    }
    
    // Générer les plots
    generate_plots(mesh, results, "plots");
    
    // ============ ANALYSE DE CONVERGENCE ============
    printf("\nPerforming convergence analysis...\n");
    printf("\n=== CONVERGENCE ANALYSIS ===\n");
    int test_sizes[] = {20, 30, 40, 50, 60};
    int n_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    // Limiter le nombre de tailles de grille si N est grand
    if (N < 60) {
        n_sizes = 3; // Prendre seulement 3 tailles
    }
    
    double** eigenvalues_grid = malloc(n_sizes * sizeof(double*));
    if (!eigenvalues_grid) {
        fprintf(stderr, "Error: Memory allocation failed for convergence analysis\n");
    } else {
        for (int s = 0; s < n_sizes; s++) {
            printf("  Testing N = %d...\n", test_sizes[s]);
            Mesh* test_mesh = create_mesh(test_sizes[s], params);
            if (!test_mesh) {
                fprintf(stderr, "Warning: Failed to create mesh for N=%d\n", test_sizes[s]);
                eigenvalues_grid[s] = NULL;
                continue;
            }
            
            SparseMatrixCSR* test_A = build_stiffness_matrix(test_mesh);
            SparseMatrixCSR* test_B = build_mass_matrix(test_mesh);
            
            if (!test_A || !test_B) {
                fprintf(stderr, "Warning: Failed to build matrices for N=%d\n", test_sizes[s]);
                free_mesh(test_mesh);
                eigenvalues_grid[s] = NULL;
                continue;
            }
            
            SolverConfig* test_config = create_solver_config(5);
            if (!test_config) {
                fprintf(stderr, "Warning: Failed to create solver config for N=%d\n", test_sizes[s]);
                free_sparse_matrix(test_A);
                free_sparse_matrix(test_B);
                free_mesh(test_mesh);
                eigenvalues_grid[s] = NULL;
                continue;
            }
            
            EigenResults* test_results = solve_eigenproblem(test_A, test_B, test_config);
            if (test_results) {
                printf("Computed %d eigenvalues:\n", test_results->n_eigenvalues);
                for (int i = 0; i < test_results->n_eigenvalues && i < 5; i++) {
                    double freq = sqrt(test_results->eigenvalues[i]) / (2 * 3.141592653589793);
                    printf("  λ%d = %.6f, f = %.3f Hz\n", i+1, test_results->eigenvalues[i], freq);
                }
            }
            
            if (test_results) {
                eigenvalues_grid[s] = malloc(5 * sizeof(double));
                if (eigenvalues_grid[s]) {
                    for (int i = 0; i < 5 && i < test_results->n_eigenvalues; i++) {
                        eigenvalues_grid[s][i] = test_results->eigenvalues[i];
                    }
                }
                free_eigen_results(test_results);
            } else {
                eigenvalues_grid[s] = NULL;
            }
            
            free_sparse_matrix(test_A);
            free_sparse_matrix(test_B);
            free_mesh(test_mesh);
            free_solver_config(test_config);
        }
        
        // Générer le plot de convergence seulement si nous avons des données
        int valid_sizes = 0;
        for (int s = 0; s < n_sizes; s++) {
            if (eigenvalues_grid[s] != NULL) valid_sizes++;
        }
        
        if (valid_sizes >= 2) {
            plot_convergence(test_sizes, eigenvalues_grid, n_sizes, 5, 
                            "plots/convergence.png");
        } else {
            printf("Insufficient data for convergence analysis\n");
        }
        
        // Libérer la mémoire
        for (int s = 0; s < n_sizes; s++) {
            if (eigenvalues_grid[s]) free(eigenvalues_grid[s]);
        }
        free(eigenvalues_grid);
    }
    
    // ============ NETTOYAGE ============
    printf("\nCleaning up...\n");
    free_sparse_matrix(A);
    free_sparse_matrix(B);
    free_mesh(mesh);
    free_membrane_params(params);
    free_solver_config(config);
    free_eigen_results(results);
    
    clock_t end_time = clock();
    double total_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    printf("\n========================================\n");
    printf("Total execution time: %.2f seconds\n", total_time);
    printf("Results saved in 'data/' and 'plots/'\n");
    printf("========================================\n");
    
    return 0;
}
