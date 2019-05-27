#include <instruccions_a_robot.h>


void moureEsq(bool direccion, uint16_t velocidad){

    param[0] = 0x20;
    param[1] = velocidad & 0xff; // Agafem els 8 bits inferiors
    param[2] = (velocidad >> 8); // Agafem els 8 bits superiors
    param[2] |= (!direccion ? 1 : 0) * BIT2;
    moureRoda(0x03, 0x03, param);

}

void moureDreta(bool direccion, uint16_t velocidad){

    param[0] = 0x20;
    param[1] = velocidad & 0xff; // Agafem els 8 bits inferiors
    param[2] = (velocidad >> 8); // Agafem els 8 bits superiors
    param[2] |= (direccion ? 1 : 0) * BIT2;
    moureRoda(0x02, 0x03, param);

}


void moureRoda(uint8_t id, uint8_t parameter_length, byte Parametros[MAX_PARAMETER_LENGTH]){

    TxPacket(id, parameter_length, 0x03, Parametros); // Cridem la funci TxPacket

}

void moure_endavant() {

    moureEsq(FORWARD, velocidad); // Velocitat de la roda esquerra val velocitat, sentit endavant
    moureDreta(FORWARD, velocidad); // Velocitat de la roda dreta val velocitat, sentit endavant

}

void moure_enrere() {

    moureEsq(BACKWARD, velocidad); // Velocitat de la roda esquerra val velocitat, sentit endarrere
    moureDreta(BACKWARD, velocidad); // Velocitat de la roda dreta val velocitat, sentit endarrere
    // dos motors mateixa direccio sentit antihorari
}

void augm_velocitat() {

    velocidad += 10;

}

void dism_velocitat() {

    velocidad -= 10;

}

void parar_robot() {

    moureDreta(FORWARD, 0); // Velocitat de la roda dreta val 0
    moureEsq(FORWARD, 0); // Velocitat de la roda esquerra val 0

}

void moure_esq() {

    moureDreta(FORWARD, velocidad);

}

void moure_dreta() {

    moureEsq(FORWARD, velocidad);

}
