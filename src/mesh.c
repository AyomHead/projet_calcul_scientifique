#include "mesh.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

Mesh* create_mesh(int N, MembraneParams* params) {
    Mesh* mesh = (Mesh*)malloc(sizeof(Mesh));
    if (!mesh) return NULL;
    
    mesh->N = N;
    mesh->total_points = N * N;
    mesh->h = DOMAIN_SIZE / (N + 1);
    
    // Allocation
    mesh->x = (double*)malloc(N * sizeof(double));
    mesh->y = (double*)malloc(N * sizeof(double));
    mesh->p_vals = (double*)malloc(mesh->total_points * sizeof(double));
    mesh->w_vals = (double*)malloc(mesh->total_points * sizeof(double));
    mesh->q_vals = (double*)malloc(mesh->total_points * sizeof(double));
    
    if (!mesh->x || !mesh->y || !mesh->p_vals || !mesh->w_vals || !mesh->q_vals) {
        free_mesh(mesh);
        return NULL;
    }
    
    // Coordonnées des points
    for (int i = 0; i < N; i++) {
        mesh->x[i] = (i + 1) * mesh->h;
        mesh->y[i] = (i + 1) * mesh->h;
    }
    
    // Calcul des coefficients
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int idx = mesh_index(i, j, mesh);
            double x = mesh_x(i, mesh);
            double y = mesh_y(j, mesh);
            
            mesh->p_vals[idx] = params->tension(x, y);
            mesh->w_vals[idx] = params->density(x, y);
            mesh->q_vals[idx] = params->potential(x, y);
        }
    }
    
    return mesh;
}

void free_mesh(Mesh* mesh) {
    if (!mesh) return;
    
    free(mesh->x);
    free(mesh->y);
    free(mesh->p_vals);
    free(mesh->w_vals);
    free(mesh->q_vals);
    free(mesh);
}

int mesh_index(int i, int j, Mesh* mesh) {
    return i * mesh->N + j;
}

double mesh_x(int i, Mesh* mesh) {
    return mesh->x[i];
}

double mesh_y(int j, Mesh* mesh) {
    return mesh->y[j];
}

void save_mesh(Mesh* mesh, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return;
    
    fprintf(file, "i,j,x,y,p,w,q\n");
    for (int i = 0; i < mesh->N; i++) {
        for (int j = 0; j < mesh->N; j++) {
            int idx = mesh_index(i, j, mesh);
            fprintf(file, "%d,%d,%.6f,%.6f,%.6f,%.6f,%.6f\n",
                   i, j, mesh->x[i], mesh->y[j],
                   mesh->p_vals[idx], mesh->w_vals[idx], mesh->q_vals[idx]);
        }
    }
    
    fclose(file);
}
