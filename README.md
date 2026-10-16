# Concentrateur de Données de Vol ARINC 429 (Temps Réel)

Passerelle logicielle temps réel entre un équipement avionique ARINC 429 simulé (développé en **C**) et un tableau de bord de supervision interactif (**Python**), utilisant la mémoire partagée POSIX et les sémaphores nommés.

---

## 📌 Présentation du Projet
Le protocole **ARINC 429** est le standard de communication série le plus répandu dans l'aviation civile (Airbus, Boeing) pour échanger des données de vol de manière sécurisée et déterministe. 

Ce projet implémente :
1. **Un producteur en C** qui décode les trames 32 bits brutes (Label, SDI, Data, SSM, Parité) et les pousse dans une zone de mémoire partagée.
2. **Un superviseur en Python** qui lit ces données en continu et applique un dictionnaire de conversion aéronautique pour afficher les grandeurs physiques en temps réel (vitesse en nœuds, altitude en pieds, cap en degrés).
3. **Un script d'automatisation (`sc.sh`)** pour orchestrer la compilation et l'affichage multi-fenêtres.

---

## 🏗️ Architecture du Système
* **C (`producteur.c` / `fonctions.c`)** : Décodage bit à bit des trames ARINC 429, contrôle d'intégrité par parité impaire et synchronisation via sémaphore POSIX (`/sem_arinc`).
* **Python (`superviseur.py`)** : Lecture binaire structurée via `mmap` et `struct`, couplée à une logique métier de conversion physique (Dictionnaire des labels).

---

## 🚀 Utilisation Rapide

Le projet intègre un script shell pour automatiser le nettoyage, la compilation et le lancement de l'application.

### 1. Cloner le dépôt
```bash
git clone git@github.com:Grace-BOUKA/Developpement-d-un-concentrateur-de-donnees-de-vol.git
cd Developpement-d-un-concentrateur-de-donnees-de-vol

### 2. Lancement du système complet
./sc.sh
