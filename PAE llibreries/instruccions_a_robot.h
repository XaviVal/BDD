#ifndef INSTRUCCIONS_A_ROBOT_H_
#define INSTRUCCIONS_A_ROBOT_H_
#include "comunicacio.h"
#include <stdbool.h>


#define MAX_PARAMETER_LENGTH 16


uint32_t velocidad;
byte param[MAX_PARAMETER_LENGTH];


void moureEsq(bool direccion, uint16_t velocidad);
void moureDreta(bool direccion, uint16_t velocidad);
void moureRoda(uint8_t id, uint8_t parameter_length, byte Parametros[MAX_PARAMETER_LENGTH]);
void moure_endavant();
void moure_enrere();
void augm_velocitat();
void dism_velocitat();
void parar_robot();
void moure_esq();
void moure_dreta();

#endif
