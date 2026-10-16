#!/bin/bash

set -e

# 1. Compilation
make clean
make all

# 2. Lancer le superviseur
gnome-terminal --geometry=80x25 -- bash -c "python superviseur.py; exec bash"

# 3. Petite pause pour laisser le temps au superviseur de s'initialiser
sleep 1

# 4. Lancer le producteur dans son propre terminal
gnome-terminal --geometry=80x25 -- bash -c "./producteur.exe; exec bash"