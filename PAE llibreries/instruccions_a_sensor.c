/*
 * lib_instrucciones_sensor.c
 *
 *  Created on: 31 maig 2018
 *      Author: mat.aules
 */

#include <instruccions_a_sensor.h>

struct RxReturn llegirSensors(void) {

    byte parametros[MAX_PARAMETER_LENGTH];

    parametros[0] = IR_IZQ;                         //Accedemos a la direccion 0x1A
    parametros[1] = 3;                              //indicamos los siguientes 3 sensores (iz,centreal,drch)
    TxPacket(100, 2, 0x02, parametros);      //enviamos la trama para que lea los valores del motor100
                                                //Comprobamos que se ha recibido correctamente
    return RxPacket();

}

void musica(void) {

    bool timeout;
    byte parametros[MAX_PARAMETER_LENGTH];
    parametros[0] = 0x28;                         //Accedemos a la direccion 0x1A
    byte notas[] = {24,7,10,8,5,5,24,10,7,7,10,24,8,5,5,10,2,6,4,24,8,24,10,7,7,10,24,8,5,5,3,8,5,5,7,24,8,10,7,8,24};
    byte tempos[] = {6,3,6,3,6,3,6,3,6,3,6,6,6,6,12,6,6,3,3,6,6,3,3,6,3,6,6,6,6,12,24,12,12,12,12,12,12,12,12,12,12};
    uint8_t i = 0;

    for(i = 0; i < 41; i++) {
        parametros[1] = notas[i];
        parametros[2] = tempos[i]/3;
        TxPacket(100, 3, 0x03, parametros);
        RxPacket();
        Reset_Timeout();

         while (true) {// tiempo en decenas de microsegundos (ara 10ms)

             timeout=TimeOut(200);
             if (timeout)break;//sale del while

         }

    }

}
