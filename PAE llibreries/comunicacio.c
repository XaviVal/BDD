/*
 *
 * INCLUDES NECESSARIS DE LA LLIBRERIA
 *
 */


#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include "lib_PAE2.h"
#include "comunicacio.h"



/*
 *
 * FUNCIONS DE LA LLIBRERIA COMUNICACI�
 *
 */


/*
 *
 *  FUNCIONS DE TRANSMISSI� (TxPacket i RxPacket)
 *
 */

byte TxPacket(byte bID, byte bParameterLength, byte bInstruction, byte Parametros[MAX_PARAMETER_LENGTH]) {

    byte bCount,bCheckSum,bPacketLength;
    byte TxBuffer[32];

    Sentit_Dades_Tx(); //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Transmetre)

    TxBuffer[0] = 0xff; //Primers 2 bytes que indiquen inici de trama FF, FF.
    TxBuffer[1] = 0xff;
    TxBuffer[2] = bID; //ID del mòdul al que volem enviar el missatge
    TxBuffer[3] = bParameterLength+2; //Length(Parameter,Instruction,Checksum)
    TxBuffer[4] = bInstruction; //Instrucció que enviem al Mòdul

    char error[] = "adr. no permitida";

    if ((Parametros[0] < 6) && (bInstruction == 3)) {//si se intenta escribir en una direccion <= 0x05,

        //emitir mensaje de error de direccion prohibida:
          halLcdPrintLine(error, 8, INVERT_TEXT);
          //y salir de la funcion sin mas:
          return 0;

    }

    for(bCount = 0; bCount < bParameterLength; bCount++) { //Comencem a generar la trama que hem d’enviar

        TxBuffer[bCount+5] = Parametros[bCount];

    }

    bCheckSum = 0;

    bPacketLength = bParameterLength+4+2;

    for(bCount = 2; bCount < bPacketLength-1; bCount++) { //Càlcul del checksum

        bCheckSum += TxBuffer[bCount];

    }

    TxBuffer[bCount] = ~bCheckSum; //Escriu el Checksum (complement a 1)

    for(bCount = 0; bCount < bPacketLength; bCount++) { //Aquest bucle és el que envia la trama al Mòdul Robot

        TxUAC2(TxBuffer[bCount]);

    }

    while( (UCA2STATW&UCBUSY)); //Espera fins que s’ha transmès el últim byte

    Sentit_Dades_Rx(); //Posem la línia de dades en Rx perquè el mòdul Dynamixel envia resposta

    return(bPacketLength);
}



struct RxReturn RxPacket(void) {

    struct RxReturn respuesta;
    byte bCount, bLenght, bChecksum, i;
    byte Rx_time_out = 0;

    Sentit_Dades_Rx(); //Ponemos la linea half duplex en Rx

    Activa_Interrupcion_TimerA1(1);

    respuesta.StatusPacket[3] = 0x00;

    for(bCount = 0; bCount < 4; bCount++) { //bRxPacketLength; bCount++)

        Reset_Timeout();
        Byte_Recibido = 0; //No_se_ha_recibido_Byte();

        while (!Byte_Recibido) { //Se_ha_recibido_Byte())

            //Byte_Recibido = has_received_byte();
            Rx_time_out = TimeOut(TIMEOUT); // tiempo en decenas de microsegundos

            if (Rx_time_out) break;//sale del while

        }

        if (Rx_time_out) break; //sale del for si ha habido Timeout
        //Si no, es que todo ha ido bien, y leemos un dato:
        respuesta.StatusPacket[bCount] = DatoLeido_UART; //Get_Byte_Leido_UART();

    }//fin del for

    if (!Rx_time_out) {// Continua llegint la resta de bytes del Status Packet

        respuesta.timeout = 0;
        bLenght = DatoLeido_UART;;

        for(bCount = 0; bCount < bLenght; bCount++) { //packet_length; i++)

            Reset_Timeout();
            Byte_Recibido = 0;

            while(!Byte_Recibido) {

                //Byte_Recibido = has_received_byte();
                Rx_time_out = TimeOut(TIMEOUT); // tiempo en decenas de microsegundos

                if (Rx_time_out) break; //sale del while

            }

            if (Rx_time_out) break; //sale del while

            respuesta.StatusPacket[bCount + 4] = DatoLeido_UART;

        }


        bChecksum = respuesta.StatusPacket[bLenght+3] + 1;

        for (i = 2; bLenght < respuesta.StatusPacket[3] + 3; i++) {

            bChecksum += respuesta.StatusPacket[bLenght];

        }

        respuesta.checksum_correct = (bChecksum == 0);

    }

    else {

        respuesta.timeout = 1;

    }

    return respuesta;

}

/*
 *
 *  FUNCIONS AUXILIARS DE RxPacket i TxPacket
 *
 */

void EUSCIA2_IRQHandler(void) { //interrupcion de recepcion en la UART A2

    UCA2IE &= ~UCRXIE; //Interrupciones desactivadas en RX
    DatoLeido_UART = UCA2RXBUF;
    Byte_Recibido = 1;
    UCA2IE |= UCRXIE; //Interrupciones reactivadas en RX

}

void Reset_Timeout(void) {

    time_g = 0; // Fem que el Time valgui 0

}

void init_timers(void) {

    /****** Timer para el timeout en RX ******/
    // Retraso de los LEDs

    TA0CTL =
        TASSEL__SMCLK + // clock SMCLK
        MC__UP +        // modo UP
        ID__8;          // /8

    TA0CCTL0 =
        CCIE; // activar int clock

    // Seteamos la constante de tiempo mximo del contador
    // Queremos que la unidad bsica sea 10^-3 segundos
    // Lo ponemos a 30 ya que f/1000/8 = 24*10^6/1000/8 = 3000
    TA0CCR0 = 3000;

}

void Sentit_Dades_Tx(void) { //Configuraci� del Half Duplex dels motors: Transmissi�

    P3OUT |= BIT0; //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Tx)

}


void Sentit_Dades_Rx(void) { //Configuraci� del Half Duplex dels motors: Recepci�

    P3OUT &= ~BIT0; //El pin P3.0 (DIRECTION_PORT) el posem a 0 (Rx)

}

void TxUAC2(byte bTxdData) {

    while(!TXD2_READY); // Espera a que estigui preparat el buffer de transmissió
    UCA2TXBUF = bTxdData;

}

void Activa_Interrupcion_TimerA1(bool enable) {

    if (enable == 0) {

        TA0CCTL0 |=  CCIE;

    }

    else {

        TA0CCTL0 &= ~CCIE;

    }
}

bool TimeOut(uint32_t t) {

    return time_g>=t;

}

void TA0_0_IRQHandler(void) {

    Activa_Interrupcion_TimerA1(0); // Desactivamos interrupciones

    time_g++; // Ha pasado 1ms

    TA0CCTL0 &= ~CCIFG; // Limpiar flag
    Activa_Interrupcion_TimerA1(1);  // Reactivamos interrupciones

}

