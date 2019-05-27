
#ifndef COMUNICACIO_H_
#define COMUNICACIO_H_

#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


/*
 *
 * DEFINES NECESSARIS DEL NOSTRE CODI
 *
 */

#define TXD2_READY (UCA2IFG & UCTXIFG)
#define MAX_PARAMETER_LENGTH 16
#define TIMEOUT 1000


/*
 *
 *TYPEDEF DE LA LLIBRERIA
 *
 */

typedef uint8_t byte;
typedef uint16_t time_t;
typedef enum {BACKWARD, FORWARD} direction_t;

typedef struct RxReturn{
    int8_t StatusPacket[MAX_PARAMETER_LENGTH];
    bool timeout;
    bool checksum_correct;
}RxReturn;


/*
 *
 * VARIABLES GLOBALS
 *
 */


uint16_t DatoLeido_UART;
bool Byte_Recibido;
byte param[MAX_PARAMETER_LENGTH];
time_t time_g;


/*
 *
 * CAPÇALERA DE LES FUNCIONS DE LA LLIBRERIA
 *
 */

byte TxPacket(byte bID, byte bParameterLength, byte bInstruction, byte Parametros[MAX_PARAMETER_LENGTH]);
struct RxReturn RxPacket(void);
void EUSCIA2_IRQHandler(void);
void Reset_Timeout(void);
void init_timers(void);
void Sentit_Dades_Tx(void);
void Sentit_Dades_Rx(void);
void TxUAC2(byte bTxdData);
void Activa_Interrupcion_TimerA1(bool enable);
bool TimeOut(uint32_t t);
void TA0_0_IRQHandler(void);

#endif /* COMUNIACIO_H_ */
