#!/bin/bash
# Script de correction des warnings

echo "🛠️  Correction du projet Membrane Solver..."

# Backup des fichiers originaux
cp src/main.c src/main.c.backup
cp src/visualization.c src/visualization.c.backup
cp Makefile Makefile.backup

# 1. Makefile
sed -i 's/CFLAGS = -O3 -march=native -fopenmp -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L/CFLAGS = -O3 -march=native -fopenmp -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -Wno-unused-result -Wno-unused-parameter/' Makefile

# 2. Corrections main.c
sed -i '
93,94s/%ld/%d/g
94s/A->n_rows/(int)A->n_rows/
94s/A->n_cols/(int)A->n_cols/
94s/A->nnz/(int)A->nnz/
107,108s/%ld/%d/g
108s/B->n_rows/(int)B->n_rows/
108s/B->n_cols/(int)B->n_cols/
108s/B->nnz/(int)B->nnz/
79s/system("mkdir -p data");/int ret = system("mkdir -p data"); (void)ret;/
149s/system("mkdir -p data");/ret = system("mkdir -p data"); (void)ret;/
150s/system("mkdir -p plots");/ret = system("mkdir -p plots"); (void)ret;/
151s/system("mkdir -p scripts");/ret = system("mkdir -p scripts"); (void)ret;/
' src/main.c

# 3. Corrections visualization.c
sed -i '
270s/%ld/%d/g
270s/mat->columns\[j\]/(int)mat->columns[j]/
314s/%ld/%d/g
314s/mat->n_rows/(int)mat->n_rows/
315s/%ld/%d/g
315s/mat->n_cols/(int)mat->n_cols/
40s/system(mkdir_cmd);/int ret = system(mkdir_cmd); (void)ret;/
126s/system(command);/ret = system(command); (void)ret;/
127s/system("mkdir -p scripts");/ret = system("mkdir -p scripts"); (void)ret;/
128s/system("mkdir -p data");/ret = system("mkdir -p data"); (void)ret;/
175s/system("mkdir -p data");/ret = system("mkdir -p data"); (void)ret;/
201s/system("mkdir -p scripts");/ret = system("mkdir -p scripts"); (void)ret;/
259s/system("mkdir -p data");/ret = system("mkdir -p data"); (void)ret;/
278s/system("mkdir -p scripts");/ret = system("mkdir -p scripts"); (void)ret;/
' src/visualization.c

echo "✅ Corrections appliquées !"
echo "📦 Backup créé: *.backup"
echo "🔧 Recompilez: make clean && make"
