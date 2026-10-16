#include <stdio.h>
#include "fonctions.h"

// Compte le nombre de bits à 1 pour vérifier la parité impaire
static bool verifier_parite_impaire(uint32_t trame) {
    int count = 0;
    for (int i = 0; i < 32; i++) {
        if ((trame >> i) & 1) {
            count++;
        }
    }
    return (count % 2 != 0); // Vrai si le nombre total de 1 est impair
}

ArincFrame decoder_trame(uint32_t trame) {
    ArincFrame f;

    // 1. Extraction des champs par décalage et masquage
    f.label    = (uint8_t)  (trame & 0xFF);
    f.sdi      = (uint8_t)  ((trame >> 8) & 0x03);
    f.data_raw = (uint32_t) ((trame >> 10) & 0x7FFFF);
    f.ssm      = (uint8_t)  ((trame >> 29) & 0x03);
    f.parite   = (uint8_t)  ((trame >> 31) & 0x01);

    // 2. Contrôle d'intégrité (Parité impaire)
    f.parite_ok = verifier_parite_impaire(trame);

    return f;
}

void afficher_trame_decodee(ArincFrame f) {
    printf("Label (Octal) : %03o (Décimal: %u)\n", f.label, f.label);
    printf("SDI           : %u\n", f.sdi);
    printf("Data (Brut)   : %u (0x%X)\n", f.data_raw, f.data_raw);
    printf("SSM           : %u ", f.ssm);
    
    switch (f.ssm) {
        case 0: printf("(Failure Warning - FW)\n"); break;
        case 1: printf("(No Computed Data - NCD)\n"); break;
        case 2: printf("(Functional Test - FT)\n"); break;
        case 3: printf("(Normal Operation - NO)\n"); break;
    }

    printf("Parité (Bit 32): %u [%s]\n", f.parite, f.parite_ok ? "VALIDE" : "ERREUR PARITÉ");
}