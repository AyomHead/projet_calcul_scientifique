import numpy as np
import matplotlib.pyplot as plt
import os

print('Generating convergence plot...')

try:
    data = np.loadtxt('data/convergence_data.csv', delimiter=',', skiprows=1)
    grid_sizes = data[:, 0]
    
    # Créer le répertoire plots s'il n'existe pas
    os.makedirs('plots', exist_ok=True)
    
    fig, axes = plt.subplots(1, 5, figsize=(15, 5))
    
    axes[0].plot(grid_sizes, data[:, 1], 'o-', linewidth=2, markersize=6)
    axes[0].set_title('Mode 1 Eigenvalue Convergence')
    axes[0].set_xlabel('Grid Size N')
    axes[0].set_ylabel('Eigenvalue')
    axes[0].grid(True, alpha=0.3)
    axes[0].set_xscale('log')
    axes[0].set_yscale('log')
    axes[1].plot(grid_sizes, data[:, 2], 'o-', linewidth=2, markersize=6)
    axes[1].set_title('Mode 2 Eigenvalue Convergence')
    axes[1].set_xlabel('Grid Size N')
    axes[1].set_ylabel('Eigenvalue')
    axes[1].grid(True, alpha=0.3)
    axes[1].set_xscale('log')
    axes[1].set_yscale('log')
    axes[2].plot(grid_sizes, data[:, 3], 'o-', linewidth=2, markersize=6)
    axes[2].set_title('Mode 3 Eigenvalue Convergence')
    axes[2].set_xlabel('Grid Size N')
    axes[2].set_ylabel('Eigenvalue')
    axes[2].grid(True, alpha=0.3)
    axes[2].set_xscale('log')
    axes[2].set_yscale('log')
    axes[3].plot(grid_sizes, data[:, 4], 'o-', linewidth=2, markersize=6)
    axes[3].set_title('Mode 4 Eigenvalue Convergence')
    axes[3].set_xlabel('Grid Size N')
    axes[3].set_ylabel('Eigenvalue')
    axes[3].grid(True, alpha=0.3)
    axes[3].set_xscale('log')
    axes[3].set_yscale('log')
    axes[4].plot(grid_sizes, data[:, 5], 'o-', linewidth=2, markersize=6)
    axes[4].set_title('Mode 5 Eigenvalue Convergence')
    axes[4].set_xlabel('Grid Size N')
    axes[4].set_ylabel('Eigenvalue')
    axes[4].grid(True, alpha=0.3)
    axes[4].set_xscale('log')
    axes[4].set_yscale('log')
    
    plt.tight_layout()
    plt.savefig('plots/convergence.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f'Convergence plot saved to {filename}')
    
except Exception as e:
    print(f'Error generating convergence plot: {e}')
