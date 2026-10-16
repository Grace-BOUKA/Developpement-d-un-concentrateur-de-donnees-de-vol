#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#include "fonctions.h"

int main(void) {
    uint32_t trame;

    printf("Renseigne la trame ARINC à traiter (Hexa ou Décimal) : ");
    if (scanf("%i", &trame) != 1) {
        fprintf(stderr, "Erreur de saisie.\n");
        return EXIT_FAILURE;
    }

    // --- CONFIGURATION DE LA MEMOIRE PARTAGEE ---
    char name[] = "/memoire_partage";
    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Failed to create a shared memory!\n");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, sizeof(ArincFrame)) == -1) {
        perror("Failed to fix the range of the shared memory\n");
        exit(EXIT_FAILURE);
    }

    ArincFrame *ptr = mmap(NULL, sizeof(ArincFrame), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Failed to link the shared memory to the process\n");
        exit(EXIT_FAILURE);
    }

    // --- DECODAGE DE LA TRAME ---
    ArincFrame frame_decodee = decoder_trame(trame);

    // --- ECRITURE DANS LA MEMOIRE PARTAGEE ---
    *ptr = frame_decodee;

    printf("===================================================\n");
    printf("Informations déchiffrées pour : 0x%08X\n", trame);
    printf("===================================================\n");
    afficher_trame_decodee(frame_decodee);
    printf("\n[C] Trame ARINC écrite dans la mémoire partagée (/dev/shm%s)\n", name);

    // --- NETTOYAGE RESSOURCES C ---
    munmap(ptr, sizeof(ArincFrame));
    close(fd);

    return EXIT_SUCCESS;
}