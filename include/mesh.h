#ifndef MESH_H
#define MESH_H
#include "membrane.h"

typedef struct {
    int N;              // Nombre de points par dimension
    int total_points;   // Total points intérieurs = N²
    double h;          // Pas spatial
    double* x;         // Coordonnées x des points
    double* y;         // Coordonnées y des points
    double* p_vals;    // Valeurs de p aux points (N²)
    double* w_vals;    // Valeurs de w aux points (N²)
    double* q_vals;    // Valeurs de q aux points (N²)
} Mesh;

// Création et destruction du maillage
Mesh* create_mesh(int N, MembraneParams* params);
void free_mesh(Mesh* mesh);

// Fonctions utilitaires
int mesh_index(int i, int j, Mesh* mesh);
double mesh_x(int i, Mesh* mesh);
double mesh_y(int j, Mesh* mesh);

// Sauvegarde du maillage
void save_mesh(Mesh* mesh, const char* filename);

#endif
