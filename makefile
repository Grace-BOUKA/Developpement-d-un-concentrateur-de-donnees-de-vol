all: producteur.exe

producteur.exe: producteur.c fonctions.c
	gcc -Wall -O2 $^ -o $@ -lpthread -lrt

clean:
	rm -f producteur.exe
