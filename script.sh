#!/bin/bash

set -e

make clean
make all
./producteur.exe
python superviseur.py