# CAHIER DES CHARGES : Concentrateur de Données de Vol ARINC 429

## 1. Contexte et Objectifs du Projet
Le projet consiste à concevoir et développer une passerelle logicielle temps réel permettant de simuler, décoder et superviser des flux de données avioniques conformes à la norme **ARINC 429**. 
L'objectif est d'offrir une architecture découplée et robuste comprenant :
* Un **producteur bas niveau en C** simulant des équipements émetteurs de trames.
* Une **mémoire partagée POSIX** couplée à un sémaphore nommé pour assurer un échange inter-processus (IPC) déterministe et sans attente active (CPU 0%).
* Un **superviseur en Python** agissant comme un tableau de bord (Cockpit Dashboard) traduisant les trames brutes en grandeurs physiques normalisées (vitesse en nœuds, altitude en pieds, cap en degrés).

---

## 2. Spécifications Fonctionnelles

### 2.1. Émission des trames (Producteur C)
* Le programme en C doit initialiser une zone de mémoire partagée (`/memoire_partage`) et un sémaphore POSIX (`/sem_arinc`).
* Il doit lire ou simuler un tableau de mots 32 bits (trames ARINC 429 normalisées).
* Pour chaque trame, il effectue :
  * L'extraction des champs par masquage et décalage (Label, SDI, Données brutes, SSM, Bit de parité).
  * Un contrôle d'intégrité (vérification de la parité impaire).
  * L'écriture de la structure `ArincFrame` dans la mémoire partagée et l'émission du sémaphore (`sem_post`).

### 2.2. Supervision et Traduction (Superviseur Python)
* Le script Python établit une liaison sécurisée avec le sémaphore et le fichier mappé en mémoire (`mmap`).
* Il se met en attente bloquante sur le sémaphore pour chaque nouvelle trame.
* À la réception, il lit la structure binaire, extrait les champs et consulte un dictionnaire de correspondance aéronautique (`ARINC_DICTIONARY`) pour convertir les données brutes (`data_raw`) en unités physiques :
  * **Label 326** : Vitesse Vraie (TAS) en nœuds (`kt`).
  * **Label 327** : Altitude / Hauteur en pieds (`ft`).
  * **Label 236** : Cap (Heading) en degrés (`°`).
* L'affichage se fait sous forme de tableau de bord rafraîchi dynamiquement.

---

## 3. Spécifications Techniques et Environnement

* **Langages** : C et Python.
* **Mécanismes IPC** : 
  * `shm_open` / `mmap` (POSIX Shared Memory).
  * `sem_open` / `sem_post` / `sem_wait` (POSIX Semaphores).
* **Automatisation** : Script Bash (`sc.sh`) gérant le nettoyage, la compilation via `Makefile` et le lancement simultané des deux processus dans des fenêtres de terminal dédiées.

---

## 4. Évolutions Futures
* **Amélioration Temps Réel Critique (Prévue)** : Implémentation d'une politique d'ordonnancement temps réel de type **`SCHED_FIFO`** sur le producteur C afin de garantir une gigue minimale et une périodicité d'émission stricte, indépendamment de la charge du système d'exploitation hôte.