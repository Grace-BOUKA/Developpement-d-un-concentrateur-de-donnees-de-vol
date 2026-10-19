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

    // Extraction des champs par décalage et masquage
    f.label    = (uint8_t)  (trame & 0xFF);
    f.sdi      = (uint8_t)  ((trame >> 8) & 0x03);

    f.data_raw = (uint32_t) ((trame >> 10) & 0x7FFFF);
    if (f.data_raw & 0x40000) {
        f.data_raw = (int32_t)(f.data_raw | 0xFFFC0000); // Extension du signe négatif
    } else {
        f.data_raw = (int32_t)f.data_raw;
    }

    f.ssm      = (uint8_t)  ((trame >> 29) & 0x03);
    f.parite   = (uint8_t)  ((trame >> 31) & 0x01);

    // Contrôle d'intégrité (Parité impaire)
    f.parite_ok = verifier_parite_impaire(trame);

    return f;
}



void afficher_trame_decodee(ArincFrame f) {
    printf("Label (Octal) : %03o (Décimal: %u)\n", f.label, f.label);
    printf("SDI           : %u\n", f.sdi);
    printf("Data (Brut)   : %d (0x%X)\n", f.data_raw, f.data_raw);
    printf("SSM           : %u ", f.ssm);
    
    switch (f.ssm) {
        case 0: printf("(Failure Warning - FW)\n"); break;
        case 1: printf("(No Computed Data - NCD)\n"); break;
        case 2: printf("(Functional Test - FT)\n"); break;
        case 3: printf("(Normal Operation - NO)\n"); break;
    }

    printf("Parité (Bit 32): %u [%s]\n", f.parite, f.parite_ok ? "VALIDE" : "ERREUR PARITÉ");
}



/* Fonction de test de fiabilité des trames ARINC 429 (Création de trame) */
uint32_t creer_trame_arinc(uint8_t label, uint8_t sdi, int32_t data_raw, uint8_t ssm) {
    uint32_t trame = 0;
    
    // 1. Label sur les 8 premiers bits (bits 1-8)
    trame |= (label & 0xFF);
    
    // 2. SDI sur 2 bits (bits 9-10)
    trame |= ((sdi & 0x03) << 8);
    
    // 3. Data sur 19 bits (bits 11-29)
    trame |= (((uint32_t)data_raw & 0x7FFFF) << 10);
    
    // 4. SSM sur 2 bits (bits 30-31)
    trame |= ((ssm & 0x03) << 29);
    
    // 5. Parité impaire (Bit 32)
    int count = 0;
    for (int i = 0; i < 31; i++) {
        if ((trame >> i) & 1) count++;
    }
    if (count % 2 == 0) {
        trame |= (1U << 31); // Force la parité à être impaire
    }
    
    return trame;
}