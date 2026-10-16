import mmap
import struct

shm_path = "/dev/shm/memoire_partage"

# B = uint8_t (label) -> 1 octet
# B = uint8_t (sdi)   -> 1 octet
# I = uint32_t (data_raw) -> 4 octets
# B = uint8_t (ssm)   -> 1 octet
# B = uint8_t (parite) -> 1 octet
# B = uint8_t (parite_ok) -> 1 octet
# Total exact : 9 octets, sans aucun décalage possible !
STRUCT_FORMAT = "<BBIBBB"
STRUCT_SIZE = struct.calcsize(STRUCT_FORMAT)

try:
    with open(shm_path, "rb") as f:
        with mmap.mmap(
            f.fileno(), STRUCT_SIZE, flags=mmap.MAP_SHARED, prot=mmap.PROT_READ
        ) as map_file:
            raw_data = map_file.read(STRUCT_SIZE)
            label, sdi, data_raw, ssm, parite, parite_ok = struct.unpack(
                STRUCT_FORMAT, raw_data
            )

            print(
                f"\n[Python] Superviseur - Trame ARINC lue depuis la SHM :"
            )
            print(f"  - Label (Octal) : {label:03o} (Décimal: {label})")
            print(f"  - SDI           : {sdi}")
            print(f"  - Data (Brut)   : {data_raw} (0x{data_raw:X})")
            print(f"  - SSM           : {ssm}")
            print(
                f"  - Parité        : {parite} ({'VALIDE' if parite_ok == 1 else 'ERREUR PARITÉ'})"
            )

except FileNotFoundError:
    print(f"Erreur : Le segment {shm_path} n'existe pas.")
except Exception as e:
    print(f"Erreur inattendue : {e}")