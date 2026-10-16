import mmap
import struct
import posix_ipc
import time

shm_path = "/dev/shm/memoire_partage"
sem_name = "/sem_arinc"

# INFORMATIONS SUR LA STRUCTURE DE LA TRAME ARINC 429
# B = uint8_t (label) -> 1 octet
# B = uint8_t (sdi)   -> 1 octet
# I = uint32_t (data_raw) -> 4 octets
# B = uint8_t (ssm)   -> 1 octet
# B = uint8_t (parite) -> 1 octet
# B = uint8_t (parite_ok) -> 1 octet
# Total exact : 9 octets, sans aucun décalage possible !
STRUCT_FORMAT = "<BBIBBB"
STRUCT_SIZE = struct.calcsize(STRUCT_FORMAT)

# Dictionnaire de traduction ARINC 429 (Label octal -> Métier physique)
ARINC_DICTIONARY = {
    0o326: {
        "nom": "Vitesse Vraie (TAS)",
        "unit": "kt",
        "scale": 0.1,  # Facteur d'échelle pour l'exemple de simulation
    },
    0o327: {
        "nom": "Altitude / Hauteur",
        "unit": "ft",
        "scale": 1.0,  # Facteur direct ou résolution standard
    },
    0o236: {
        "nom": "Cap (Heading)",
        "unit": "°",
        "scale": 0.05, # Facteur d'échelle pour simuler des degrés
    },
}



# Connexion sécurisée avec attente si le producteur n'est pas encore prêt
semaphore = None
f = None
while semaphore is None or f is None:
    try:
        if semaphore is None:
            # Connexion au sémaphore nommé créé par le C
            # flags=0 signifie qu'on s'attend à ce qu'il existe déjà
            semaphore = posix_ipc.Semaphore(sem_name, flags=0)
        if f is None:
            f = open(shm_path, "rb")
    except (posix_ipc.ExistentialError, FileNotFoundError):
        time.sleep(0.5) # Réessaie toutes les 0.5 secondes


print("[Python] Connexion établie ! En attente des trames ARINC...\n", flush=True)

try:
    
    with f:
        with mmap.mmap(
            f.fileno(), STRUCT_SIZE, flags=mmap.MAP_SHARED, prot=mmap.PROT_READ
        ) as map_file:
            
            while True:
                
                # Attente bloquante du signal du producteur C (équivalent de sem_wait)
                # Le script se met en pause ici sans consommer de CPU tant que le C n'écrit rien.
                semaphore.acquire()
                
                # Nettoie l'écran et remet le curseur en haut du terminal
                print("\033[H\033[J", end="", flush=True)
                
                # Lecture immédiate de la mémoire partagée
                map_file.seek(0)
                raw_data = map_file.read(STRUCT_SIZE)
                
                label, sdi, data_raw, ssm, parite, parite_ok = struct.unpack(
                    STRUCT_FORMAT, raw_data
                )

                print(
                    f"\n[Python] Superviseur - Trame ARINC recue et synchronisée depuis la SHM :"
                )
                print(f"  - Label (Octal) : {label:03o} (Décimal: {label})")
                print(f"  - SDI           : {sdi}")
                print(f"  - Data (Brut)   : {data_raw} (0x{data_raw:X})")
                print(f"  - SSM           : {ssm}")
                print(
                    f"  - Parité        : {parite} ({'VALIDE' if parite_ok == 1 else 'ERREUR PARITÉ'})"
                )
                
                # Traduction physique via le dictionnaire
                print("-" * 60)
                if label in ARINC_DICTIONARY:
                    info = ARINC_DICTIONARY[label]
                    # Calcul de la grandeur physique
                    valeur_physique = data_raw * info["scale"]
                    print(f"✈️  PARAMÈTRE DÉCODÉ : {info['nom']}")
                    print(f"📊 VALEUR PHYSIQUE  : {valeur_physique:.2f} {info['unit']}")
                else:
                    print(f"⚠️  Label {label:03o} non répertorié dans le dictionnaire.")
                print("-" * 60)

except posix_ipc.ExistentialError:
    print(f"Erreur : Le sémaphore {sem_name} ou la mémoire partagée n'existe pas. Lancez d'abord le C !")
except KeyboardInterrupt:
    print("\n[Python] Arrêt du superviseur.")
except FileNotFoundError:
    print(f"Erreur : Le segment {shm_path} n'existe pas.")
except Exception as e:
    print(f"Erreur inattendue : {e}")