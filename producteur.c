#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#include "fonctions.h"
#include<pthread.h>
#include <semaphore.h>
#include<time.h>

#define FREQUENCE 800 // 800 ms (Pour la visualisation)

/* Memoire de partage */
char shm_name[] = "/memoire_partage";
char sem_name[] = "/sem_arinc";
sem_t *sem=NULL; /* Semaphore nommé pour la synchronisation */


/* Trames ARINC 429 à décoder */
uint32_t trames_simulees[] = {
0x603841CA,0x601C21CA,0x600191C8,0x7FFF61C8,0xE00169D0,0xE0271083,
0xE09C4083,0xE203A083,0x6005DC88,0x7FFC1888,0x60190185,0x60271185,
0xE02BC186,0x60433186,0x6046510D,0x608598E0,0xE08FC0E0,0x608CA0E1,
0xE00A28E2,0xE00708E8,0x6001408A,0x7FFF108A,0x6003E80B,0x600FA1DB,
0xE01389DB,0xE8AB8A08,0xF126F209,0x003841CA,0x60134828,0xE058B87F
};
ArincFrame *ptr=NULL; /* Pointeur sur la donnée à partagée */


/*============================== PRODUCTEUR DE TRAME C ================================*/

void* thread_producteur_arinc(void* args)
{
    /* ------------- CONFIGURATION TEMPS REEL --------------*/
    struct sched_param param;
    param.sched_priority=5;

    // -----> Passage en ordonnancement temps réel
    if(pthread_setschedparam(pthread_self(),SCHED_FIFO,&param) !=0)
    {
        perror("Echec d'ordonnacement! (programme lancé en mode super-user?)");
        exit(EXIT_FAILURE);
    }


    // Préparation pour nanosleep
    struct timespec attente = {0, FREQUENCE * 1000000};

    /*--------------- BOUCLE DE SIMULATION ------------------*/
    int i=0;
    while (1) {
        // Nettoie l'écran et remet le curseur en haut
        printf("\033[H\033[J");

        uint32_t trame_courante = trames_simulees[i];
        
        // --- DECODAGE DE LA TRAME ---
        ArincFrame frame_decodee= decoder_trame(trame_courante);
        printf("=============================================================\n");
        printf("Informations déchiffrées pour : 0x%08X\n", trame_courante);
        printf("=============================================================\n");
        printf("\n[C] Trame ARINC écrite dans la mémoire partagée (/dev/shm%s)\n", shm_name);
        afficher_trame_decodee(frame_decodee);

        // --- ECRITURE DANS LA MEMOIRE PARTAGEE ---
        *ptr = frame_decodee;

        // --- SIGNAL AU SUPERVISUER ---
        sem_post(sem);
        i++;

        if(i>=30) i=0;

        nanosleep(&attente,NULL);

    }

    pthread_exit(NULL);
}




/*============================================== BOUCLE PRINCIPALE ==================================================*/
int main(void) 
{   
    // // ----- NETTOYAGE PRÉVENTIF -----
    // shm_unlink(shm_name);
    // sem_unlink(sem_name);
    // Label 0o312 (qui fait 202 en décimal), SDI = 1, Data = 3600, SSM = 3 (Normal Operation)

    srand(time(NULL));


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
    ptr = mmap(NULL, sizeof(ArincFrame), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Echec addressage de la mémoire partagée!\n");
        exit(EXIT_FAILURE);
    }


    // ------ SEMAPHORE POSIX (synchronisation) ------
    sem = sem_open(sem_name, O_CREAT, 0666, 0);  // O_CREAT : crée le sémaphore s'il n'existe pas, valeur initiale à 0
    if (sem == SEM_FAILED) {
        perror("Erreur sem_open");
        exit(EXIT_FAILURE);
    }


    printf("----> [C] Producteur lancé avec sémaphore...<----\n");

    /*=========================================== BOUCLE DE SIMULATION ============================================*/
    pthread_t th_producteur;
    pthread_create(&th_producteur,NULL,&thread_producteur_arinc,NULL); /* Création et lancement de la tâche */
    pthread_join(th_producteur,NULL);
    /*=============================================================================================================*/


    // --- NETTOYAGE RESSOURCES C ---
    munmap(ptr, sizeof(ArincFrame));
    close(fd);


    // DESTRUCTION FINALE (Suppréssion des mémoires patagées du système)
    shm_unlink(shm_name);
    sem_unlink(sem_name);

    return EXIT_SUCCESS;
}