import numpy as np
import matplotlib.pyplot as plt
import os

print('Generating matrix sparsity plot...')

try:
    data = np.loadtxt('data/matrix_pattern.csv', delimiter=',', skiprows=1)
    rows = data[:, 0].astype(int)
    cols = data[:, 1].astype(int)
    
    # Créer le répertoire plots s'il n'existe pas
    os.makedirs('plots', exist_ok=True)
    
    plt.figure(figsize=(10, 10))
    
    # Tracer les points non nuls
    plt.scatter(cols, rows, s=0.5, c='blue', alpha=0.6, marker='s')
    
    plt.title('Matrix Sparsity Pattern', fontsize=14)
    plt.xlabel('Column Index', fontsize=12)
    plt.ylabel('Row Index', fontsize=12)
    plt.gca().invert_yaxis()
    plt.grid(True, alpha=0.2)
    plt.axis('equal')
    
    # Ajouter des informations sur la matrice
    n_rows = 2500
    n_cols = 2500
    nnz = len(rows)
    sparsity = (1.0 - nnz/(n_rows*n_cols)) * 100
    
    info_text = f'Dimensions: {n_rows} x {n_cols}\nNon-zeros: {nnz}\nSparsity: {sparsity:.2f}%'
    plt.figtext(0.02, 0.98, info_text, fontsize=10,
                verticalalignment='top',
                bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
    
    plt.tight_layout()
    plt.savefig('plots/matrix_sparsity.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f'Sparsity plot saved to {filename}')
    
except Exception as e:
    print(f'Error generating sparsity plot: {e}')
