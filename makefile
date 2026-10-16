all: producteur.exe

producteur.exe: producteur.c fonctions.c
	gcc -Wall -O2 $^ -o $@

clean:
	rm -f producteur.exe
