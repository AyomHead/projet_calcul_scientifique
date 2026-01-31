# Membrane Vibration Solver (Version Simple)

Solveur de valeurs propres pour une membrane vibrante avec coefficients variables.
Utilise **DSYGV** (dense solver) d'Intel MKL.

## 🚀 Installation rapide

```bash
# 1. Compiler
make all

# 2. Installer dépendances Python (si besoin)
make install-py-deps

# 3. Exécuter
make run

# Ou avec paramètres:
./bin/membrane_solver 40 8
