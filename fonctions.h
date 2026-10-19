#include <stdint.h>
#include <stdbool.h>


/* Attribut */
typedef struct {
    uint8_t  label;     // Stocké en valeur numérique (afficher en octal %03o)
    uint8_t  sdi;       // 0 à 3
    int32_t data_raw;  // 19 bits de données brutes
    uint8_t  ssm;       // 0 à 3
    uint8_t  parite;     // 0 ou 1
    uint8_t  parite_ok; // true si la parité est valide (nombre impair de 1)
} __attribute__((packed)) ArincFrame; // <-- Obligatoire pour le passage en Python!

// Prototypes
ArincFrame decoder_trame(uint32_t trame);
void afficher_trame_decodee(ArincFrame frame);
uint32_t creer_trame_arinc(uint8_t label, uint8_t sdi, int32_t data_raw, uint8_t ssm);
