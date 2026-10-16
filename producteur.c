#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#include "fonctions.h"
#include<semaphore.h>

char shm_name[] = "/memoire_partage";
char sem_name[] = "/sem_arinc";

int main(void) 
{   
    // // ----- NETTOYAGE PRÉVENTIF -----
    // shm_unlink(shm_name);
    // sem_unlink(sem_name);


    // --- CONFIGURATION DE LA MEMOIRE PARTAGEE ---
    int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Echec de création de la mémoire de partage!\n");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(fd, sizeof(ArincFrame)) == -1) {
        perror("Echec allocation de la memoire de partage!\n");
        exit(EXIT_FAILURE);
    }
    ArincFrame *ptr = mmap(NULL, sizeof(ArincFrame), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Echec addressage de la mémoire partagée!\n");
        exit(EXIT_FAILURE);
    }


    // --- SEMAPHORE POSIX (synchronisation) ---
    char sem_name[] = "/sem_arinc";
    // O_CREAT : crée le sémaphore s'il n'existe pas, valeur initiale à 0
    sem_t *sem = sem_open(sem_name, O_CREAT, 0666, 0);
    if (sem == SEM_FAILED) {
        perror("Erreur sem_open");
        exit(EXIT_FAILURE);
    }

    printf("----> [C] Producteur lancé avec sémaphore...<----\n");

    /* ==================================== BOUCLE DE SIMULATION =====================================*/

    uint32_t trames_simulees[] = {0x0447A9D7, 0x022DAD9E, 0x0CC372D6};
    int i=0;
    while (1) {
        // Nettoie l'écran et remet le curseur en haut
        printf("\033[H\033[J");

        uint32_t trame_courante = trames_simulees[i % 3];
        
        // --- DECODAGE DE LA TRAME ---
        ArincFrame frame_decodee= decoder_trame(trame_courante);
        printf("============================================================\n");
        printf("Informations déchiffrées pour : 0x%08X\n", trame_courante);
        printf("============================================================\n");
        printf("\n[C] Trame ARINC écrite dans la mémoire partagée (/dev/shm%s)\n", shm_name);
        afficher_trame_decodee(frame_decodee);

        // --- ECRITURE DANS LA MEMOIRE PARTAGEE ---
        *ptr = frame_decodee;

        // --- SIGNAL AU SUPERVISUER ---
        sem_post(sem);

        i++;
        usleep(1000000); // 1 seconde

    }

    // --- NETTOYAGE RESSOURCES C ---
    munmap(ptr, sizeof(ArincFrame));
    close(fd);

    // DESTRUCTION FINALE (Suppression des noms du système)
    shm_unlink(shm_name);
    sem_unlink(sem_name);

    return EXIT_SUCCESS;
}