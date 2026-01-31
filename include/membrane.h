#ifndef MEMBRANE_H
#define MEMBRANE_H

#include <mkl/mkl.h>


// Constantes physiques
#define DOMAIN_SIZE 1.0
#define PI 3.14159265358979323846

// Structure pour les paramètres de la membrane
typedef struct {
    double (*tension)(double x, double y);    // Fonction p(x,y)
    double (*density)(double x, double y);    // Fonction w(x,y)
    double (*potential)(double x, double y);  // Fonction q(x,y)
    double obstacle_center_x;
    double obstacle_center_y;
    double obstacle_strength;
    double obstacle_width;
} MembraneParams;

// Structure pour les résultats
typedef struct {
    int n_eigenvalues;          // Nombre de valeurs propres calculées
    double* eigenvalues;        // Valeurs propres
    double** eigenvectors;      // Vecteurs propres (matrice N² x k)
    double* residuals;          // Résidus
    double computation_time;    // Temps de calcul
    int iterations;            // Nombre d'itérations
} EigenResults;

// Fonctions pour les coefficients
double default_tension(double x, double y);
double default_density(double x, double y);
double default_potential(double x, double y);

// Initialisation/liberation des paramètres
MembraneParams* create_default_params();
void free_membrane_params(MembraneParams* params);

#endif
