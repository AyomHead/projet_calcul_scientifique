# Makefile pour Membrane Solver avec MKL - Version Simple DSYGV
CC = gcc
CFLAGS = -O3 -march=native -fopenmp -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -Wno-unused-result -Wno-unused-parameter
INCLUDES = -I./include -I/usr/include
LIBS = -lmkl_rt -lpthread -lm -ldl

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Fichiers source
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/membrane_solver

# Cible par défaut
all: directories $(TARGET)

# Création des répertoires
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) data plots scripts

# Compilation des fichiers objet
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Édition des liens
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)
	@echo "✓ Compilation réussie: $(TARGET)"

# Nettoyage
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@rm -f data/*.csv plots/*.png scripts/*.py
	@echo "✓ Nettoyage effectué"

# Exécution
run: $(TARGET)
	@echo "=== Exécution du programme ==="
	@./$(TARGET)

# Exécution avec paramètres
run-test: $(TARGET)
	@echo "=== Test avec N=30, 5 modes ==="
	@./$(TARGET) 30 5

# Installation des dépendances Python
install-py-deps:
	@echo "=== Installation des dépendances Python ==="
	@python3 -m pip install --upgrade pip
	@python3 -m pip install numpy matplotlib
	@echo "✓ Dépendances Python installées"

# Vérification MKL
check-mkl:
	@echo "=== Vérification MKL ==="
	@if [ -f "/usr/include/mkl/mkl.h" ] || [ -f "/usr/include/mkl.h" ]; then \
		echo "✓ mkl.h trouvé"; \
	else \
		echo "✗ mkl.h NON trouvé"; \
		echo "Installez MKL: sudo apt-get install intel-mkl-2020.0-088"; \
	fi
	@if ldconfig -p | grep -q libmkl_rt; then \
		echo "✓ libmkl_rt.so trouvée"; \
	else \
		echo "✗ libmkl_rt.so NON trouvée"; \
	fi

# Aide
help:
	@echo "Commandes disponibles:"
	@echo "  make all          - Compiler le programme"
	@echo "  make run          - Exécuter le programme"
	@echo "  make run-test     - Exécuter avec paramètres de test"
	@echo "  make clean        - Nettoyer les fichiers générés"
	@echo "  make check-mkl    - Vérifier l'installation MKL"
	@echo "  make install-py-deps - Installer dépendances Python"
	@echo ""
	@echo "Utilisation: ./bin/membrane_solver [N] [modes]"
	@echo "  N:     Taille de grille (défaut: 50)"
	@echo "  modes: Nombre de modes à calculer (défaut: 10)"

.PHONY: all clean run run-test check-mkl install-py-deps help directories
