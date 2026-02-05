# Python script for plotting membrane modes
import numpy as np
import matplotlib.pyplot as plt
import os
from mpl_toolkits.mplot3d import Axes3D

# Read data
def read_mode_data(filename):
    try:
        data = np.loadtxt(filename, delimiter=',')
        return data
    except Exception as e:
        print(f'Error reading {filename}: {e}')
        return None

# Plot function
def plot_mode(data, mode_num):
    if data is None:
        print(f'Skipping mode {mode_num}: No data')
        return
    
    fig = plt.figure(figsize=(10, 8))
    ax = fig.add_subplot(111, projection='3d')
    
    x = data[:, 0]
    y = data[:, 1]
    z = data[:, 2]
    
    # Reshape pour la grille
    n = int(np.sqrt(len(x)))
    X = x.reshape((n, n))
    Y = y.reshape((n, n))
    Z = z.reshape((n, n))
    
    surf = ax.plot_surface(X, Y, Z, cmap='RdBu_r',
                           linewidth=0, antialiased=True)
    fig.colorbar(surf, ax=ax, shrink=0.5, aspect=5)
    ax.set_title(f'Membrane Mode {mode_num}')
    ax.set_xlabel('X coordinate')
    ax.set_ylabel('Y coordinate')
    ax.set_zlabel('Amplitude')
    
    # Créer le répertoire plots s'il n'existe pas
    os.makedirs('plots', exist_ok=True)
    
    plt.savefig(f'plots/mode_{mode_num:02d}.png', dpi=300, bbox_inches='tight')
    print(f'Generated plot for mode {mode_num}')
    plt.close()

# Main execution
if __name__ == '__main__':
    print('Generating membrane mode plots...')
    
    # Créer le répertoire data s'il n'existe pas
    os.makedirs('data', exist_ok=True)
    
    # Par défaut, on génère les 5 premiers modes
    for i in range(1, 6):
        filename = f'data/mode_{i:02d}.csv'
        if os.path.exists(filename):
            data = read_mode_data(filename)
            plot_mode(data, i)
        else:
            print(f'File {filename} not found, skipping mode {i}')
    print('Plot generation complete!')
