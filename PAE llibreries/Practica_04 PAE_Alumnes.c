/******************************
 *
 * Practica_04_PAE Programaci� de Ports
 * i pr�ctica de les instruccions de control de flux:
 * "do ... while", "switch ... case", "if" i "for"
 * UB, 02/2017.
 *****************************/

#include <moure_robot.h>
#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include "lib_PAE2.h" //Libreria grafica + configuracion reloj MSP432


//Definimos los dos botones y el joystick de la placa
#define Button_S1 1
#define Button_S2 2
#define Jstick_Left 3
#define Jstick_Right 4
#define Jstick_Up 5
#define Jstick_Down 6
#define Jstick_Center 7

//Definimos uart estado
//typedef uint8_t byte;
#define TXD2_READY (UCA2IFG & UCTXIFG)

char saludo[16] = " MENU";//max 15 caracteres visibles
char cadena[16];
char cadena1[32];
char borrado[] = "               "; //una linea entera de 15 espacios en blanco
uint8_t linea = 0;
uint8_t estado=0;
uint8_t estado_anterior = 8;
bool sentido = true;//establecemos el sentido de las ruedas por defecto hacia adelante
uint8_t menu = 0;
uint8_t selectedSubMenus = 0;
bool mov = false, vel = false;

void accionesMenu(uint8_t);
void imprimirMenu(uint8_t);
void menuModo();
void menuCreditos();
void menuPared();
void menuSensores();

/*----------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/

/**************************************************************************
 * INICIALIZACI�N DEL CONTROLADOR DE INTERRUPCIONES (NVIC).
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
void init_interrupciones(){
    // Configuracion al estilo MSP430 "clasico":
    // Enable Port 4 interrupt on the NVIC
    // segun datasheet (Tabla "6-12. NVIC Interrupts", capitulo "6.6.2 Device-Level User Interrupts", p80-81 del documento SLAS826A-Datasheet),
    // la interrupcion del puerto 4 es la User ISR numero 38.
    // Segun documento SLAU356A-Technical Reference Manual, capitulo "2.4.3 NVIC Registers"
    // hay 2 registros de habilitacion ISER0 y ISER1, cada uno para 32 interrupciones (0..31, y 32..63, resp.),
    // accesibles mediante la estructura NVIC->ISER[x], con x = 0 o x = 1.
    // Asimismo, hay 2 registros para deshabilitarlas: ICERx, y dos registros para limpiarlas: ICPRx.

    //Int. port 3 = 37 corresponde al bit 5 del segundo registro ISER1:
    NVIC->ICPR[1] |= BIT5; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT5; //y habilito las interrupciones del puerto
    //Int. port 4 = 38 corresponde al bit 6 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT6; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT6; //y habilito las interrupciones del puerto
    //Int. port 5 = 39 corresponde al bit 7 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT7; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT7; //y habilito las interrupciones del puerto

    //NVIC -> ISER[0] |= BIT8; //interrupcion timerA
    NVIC->ICPR[0] |= BITA;//Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[0] |= BITA; //interrupcion timer1

    NVIC->ICPR[0] |= BIT(18);//Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[0] |= BIT(18); //interrupcion UART2

    __enable_interrupt(); //Habilitamos las interrupciones a nivel global del micro.
}

/**************************************************************************
 * INICIALIZACI�N DE LA PANTALLA LCD.
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
void init_LCD(void)
{
    halLcdInit(); //Inicializar y configurar la pantallita
    halLcdClearScreenBkg(); //Borrar la pantalla, rellenando con el color de fondo
}

void init_UART(void)
{
    UCA2CTLW0 |= UCSWRST; //Fem un reset de la USCI, desactiva la USCI
    UCA2CTLW0 |= UCSSEL__SMCLK; //UCSYNC=0 mode as�ncron
    //UCMODEx=0 seleccionem mode UART
    //UCSPB=0 nomes 1 stop bit
    //UC7BIT=0 8 bits de dades
    //UCMSB=0 bit de menys pes primer
    //UCPAR=x ja que no es fa servir bit de paritat
    //UCPEN=0 sense bit de paritat
                        //Triem SMCLK (16MHz) com a font del clock BRCLK
    UCA2MCTLW = UCOS16; // Necessitem sobre-mostreig => bit 0 = UCOS16 = 1
    UCA2BRW = 3; //Prescaler de BRCLK fixat a 3. Com SMCLK va a24MHz,
                        //volem un baud rate de 500kb/s i fem sobre-mostreig de 16
                        //el rellotge de la UART ha de ser de 8MHz (24MHz/3).
                        //Configurem els pins de la UART
    P3SEL0 |= BIT2 | BIT3; //I/O funci�: P3.3 = UART2TX, P3.2 = UART2RX
    P3SEL1 &= ~ (BIT2 | BIT3);
                        //Configurem pin de selecci� del sentit de les dades Transmissi�/Recepeci�
    P3SEL0 &= ~BIT0; //Port P3.0 com GPIO
    P3SEL1 &= ~BIT0;
    P3DIR |= BIT0; //Port P3.0 com sortida (Data Direction selector Tx/Rx)
    P3OUT &= ~BIT0; //Inicialitzem Sentit Dades a 0 (Rx)
    UCA2CTLW0 &= ~UCSWRST; //Reactivem la l�nia de comunicacions s�rie
    UCA2IE |= UCRXIE; //Aix� nom�s s�ha d�activar quan tinguem la rutina de recepci�
}




/**************************************************************************
 * BORRAR LINEA
 *
 * Datos de entrada: Linea, indica la linea a borrar
 *
 * Sin datos de salida
 *
 **************************************************************************/
void borrar(uint8_t Linea)
{
	halLcdPrintLine(borrado, Linea, NORMAL_TEXT); //escribimos una linea en blanco
}

/**************************************************************************
 * ESCRIBIR LINEA
 *
 * Datos de entrada: Linea, indica la linea del LCD donde escribir
 * 					 String, la cadena de caracteres que vamos a escribir
 *
 * Sin datos de salida
 *
 **************************************************************************/
void escribir(char String[], uint8_t Linea)

{
	halLcdPrintLine(String, Linea, NORMAL_TEXT); //Enviamos la String al LCD, sobreescribiendo la Linea indicada.
}

/**************************************************************************
 * INICIALIZACI�N DE LOS BOTONES & LEDS DEL BOOSTERPACK MK II.
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
void init_botons(void)
{
    //Configuramos botones y leds
    //***************************
    //Leds RGB del MK II:
      P2DIR |= 0x50;  //Pines P2.4 (G), 2.6 (R) como salidas Led (RGB)
      P5DIR |= 0x40;  //Pin P5.6 (B)como salida Led (RGB)
      P2OUT &= 0xAF;  //Inicializamos Led RGB a 0 (apagados)
      P5OUT &= ~0x40; //Inicializamos Led RGB a 0 (apagados)

    //7 Leds
      P7DIR |= 0xFF;  //Pines P7.0 ... P7.7 como salida de leds de la placa secundaria
      P7OUT &= ~0xFF;   //Inicializamos el P7 a 0 (apagados)

    //Boton S1 del MK II:
      P5SEL0 &= ~0x02;   //Pin P5.1 como I/O digital,
      P5SEL1 &= ~0x02;   //Pin P5.1 como I/O digital,
      P5DIR &= ~0x02; //Pin P5.1 como entrada
      P5IES &= ~0x02;   // con transicion L->H
      P5IE |= 0x02;     //Interrupciones activadas en P5.1,
      P5IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 5
      //P5REN: Ya hay una resistencia de pullup en la placa MK II

    //Boton S2 del MK II:
      P3SEL0 &= ~0x20;   //Pin P3.5 como I/O digital,
      P3SEL1 &= ~0x20;   //Pin P3.5 como I/O digital,
      P3DIR &= ~0x20; //Pin P3.5 como entrada
      P3IES &= ~0x20;   // con transicion L->H
      P3IE |= 0x20;   //Interrupciones activadas en P3.5
      P3IFG = 0;  //Limpiamos todos los flags de las interrupciones del puerto 3
      //P3REN: Ya hay una resistencia de pullup en la placa MK II

    //Configuramos los GPIOs del joystick del MK II:
      P4DIR &= ~(BIT1 + BIT5 + BIT7);   //Pines P4.1, 4.5 y 4.7 como entrades,
      P4SEL0 &= ~(BIT1 + BIT5 + BIT7);  //Pines P4.1, 4.5 y 4.7 como I/O digitales,
      P4SEL1 &= ~(BIT1 + BIT5 + BIT7);
      P4REN |= BIT1 + BIT5 + BIT7;  //con resistencia activada
      P4OUT |= BIT1 + BIT5 + BIT7;  // de pull-up
      P4IE |= BIT1 + BIT5 + BIT7;   //Interrupciones activadas en P4.1, 4.5 y 4.7,
      P4IES &= ~(BIT1 + BIT5 + BIT7);   //las interrupciones se generaran con transicion L->H
      P4IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 4

      P5DIR &= ~(BIT4 + BIT5);  //Pines P5.4 y 5.5 como entrades,
      P5SEL0 &= ~(BIT4 + BIT5); //Pines P5.4 y 5.5 como I/O digitales,
      P5SEL1 &= ~(BIT4 + BIT5);
      P5IE |= BIT4 + BIT5;  //Interrupciones activadas en 5.4 y 5.5,
      P5IES &= ~(BIT4 + BIT5);  //las interrupciones se generaran con transicion L->H
      P5IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 4
    // - Ya hay una resistencia de pullup en la placa MK II
}

void seleccionaMenu(uint8_t selected){
    switch(menu){
    case 0:
        imprimirMenu(selected);
        break;
    case 1:
        menuModo();
        break;
    case 2:
        menuPared();
        break;
    case 3:
        menuSensores();
        break;
    case 4:
        menuCreditos();
        break;
    }
}

void imprimirMenu(uint8_t selected){
    uint8_t i = 2;
    for (i = 2; i < 10; i++){
        borrar(i);
    }
    linea = 2;
    switch(selected){
    case 0:
        sprintf(cadena, "MODO");
        halLcdPrintLine(cadena,linea++, INVERT_TEXT);
        sprintf(cadena, "PARED");
        escribir(cadena,linea++);
        sprintf(cadena, "SENSORES");
        escribir(cadena,linea++);
        sprintf(cadena, "CREDITOS");
        escribir(cadena,linea++);
        sprintf(cadena, "RUN");
        escribir(cadena,linea++);
        break;
    case 1:
        sprintf(cadena, "MODO");
        escribir(cadena,linea++);
        sprintf(cadena, "PARED");
        halLcdPrintLine(cadena,linea++, INVERT_TEXT);
        sprintf(cadena, "SENSORES");
        escribir(cadena,linea++);
        sprintf(cadena, "CREDITOS");
        escribir(cadena,linea++);
        sprintf(cadena, "RUN");
        escribir(cadena,linea++);
        break;
    case 2:
        sprintf(cadena, "MODO");
        escribir(cadena,linea++);
        sprintf(cadena, "PARED");
        escribir(cadena,linea++);
        sprintf(cadena, "SENSORES");
        halLcdPrintLine(cadena,linea++, INVERT_TEXT);
        sprintf(cadena, "CREDITOS");
        escribir(cadena,linea++);
        sprintf(cadena, "RUN");
        escribir(cadena,linea++);
        break;
    case 3:
        sprintf(cadena, "MODO");
        escribir(cadena,linea++);
        sprintf(cadena, "PARED");
        escribir(cadena,linea++);
        sprintf(cadena, "SENSORES");
        escribir(cadena,linea++);
        sprintf(cadena, "CREDITOS");
        halLcdPrintLine(cadena,linea++, INVERT_TEXT);
        sprintf(cadena, "RUN");
        escribir(cadena,linea++);
        break;
    case 4:
        sprintf(cadena, "MODO");
        escribir(cadena,linea++);
        sprintf(cadena, "PARED");
        escribir(cadena,linea++);
        sprintf(cadena, "SENSORES");
        escribir(cadena,linea++);
        sprintf(cadena, "CREDITOS");
        escribir(cadena,linea++);
        sprintf(cadena, "RUN");
        halLcdPrintLine(cadena,linea++, INVERT_TEXT);
        break;
    }

}

void menuPared(){
    uint8_t i = 2;
    for (i = 2; i < 10; i++){
        borrar(i);
    }
    linea = 2;
    switch(selectedSubMenus){
        case 0:
            sprintf(cadena, "AUTOMATICO");
            halLcdPrintLine(cadena,linea++, INVERT_TEXT);
            sprintf(cadena, "IZQUIERDA");
            escribir(cadena,linea++);
            sprintf(cadena, "DERECHA");
            escribir(cadena,linea++);
            break;
        case 1:
            sprintf(cadena, "AUTOMATICO");
            escribir(cadena,linea++);
            sprintf(cadena, "IZQUIERDA");
            halLcdPrintLine(cadena,linea++, INVERT_TEXT);
            sprintf(cadena, "DERECHA");
            escribir(cadena,linea++);
            break;
        case 2:
            sprintf(cadena, "AUTOMATICO");
            escribir(cadena,linea++);
            sprintf(cadena, "IZQUIERDA");
            escribir(cadena,linea++);
            sprintf(cadena, "DERECHA");
            halLcdPrintLine(cadena,linea++, INVERT_TEXT);
            break;
    }
}

void menuModo(){
    uint8_t i = 2;
    for (i = 2; i < 10; i++){
        borrar(i);
    }
    linea = 2;
    switch(selectedSubMenus){
        case 0:
            sprintf(cadena, "VELOCIDAD %d", setvelocidad());
            halLcdPrintLine(cadena,linea++, INVERT_TEXT);
            sprintf(cadena, "MODO DISCO %d",getDiscoMode() );
            escribir(cadena,linea++);
            sprintf(cadena, "INTERMITENTES %d",getIntermitentes());
            escribir(cadena,linea++);
            break;
        case 1:
            sprintf(cadena, "VELOCIDAD %d", setvelocidad());
            escribir(cadena,linea++);
            sprintf(cadena, "MODO DISCO %d", getDiscoMode() );
            halLcdPrintLine(cadena,linea++, INVERT_TEXT);
            sprintf(cadena, "INTERMITENTES %d", getIntermitentes() );
            escribir(cadena,linea++);
            break;
        case 2:
            sprintf(cadena, "VELOCIDAD %d", setvelocidad() );
            escribir(cadena,linea++);
            sprintf(cadena, "MODO DISCO %d", getDiscoMode() );
            escribir(cadena,linea++);
            sprintf(cadena, "INTERMITENTES %d", getIntermitentes());
            halLcdPrintLine(cadena,linea++, INVERT_TEXT);
            break;
        default:
            sprintf(cadena, "VELOCIDAD %d", setvelocidad() );
            escribir(cadena,linea++);
            sprintf(cadena, "MODO DISCO %d", getDiscoMode() );
            escribir(cadena,linea++);
            sprintf(cadena, "INTERMITENTES %d", getIntermitentes());
            halLcdPrintLine(cadena,linea++, INVERT_TEXT);
            break;
    }
}

void menuSensores(){
    uint8_t i = 2;
    for (i = 2; i < 10; i++){
        borrar(i);
    }
    linea = 2;
    switch(selectedSubMenus){
        case 0:
            sprintf(cadena, "CENTRAL %d", getDisMaxC());
            halLcdPrintLine(cadena,linea++, INVERT_TEXT);
            sprintf(cadena, "DERECHO %d",getDisMaxDr());
            escribir(cadena,linea++);
            sprintf(cadena, "IZQUIERDO %d",getDisMaxIz());
            escribir(cadena,linea++);
            break;
        case 1:
            sprintf(cadena, "CENTRAL %d", getDisMaxC());
            escribir(cadena,linea++);
            sprintf(cadena, "DERECHO %d",getDisMaxDr());
            halLcdPrintLine(cadena,linea++, INVERT_TEXT);
            sprintf(cadena, "IZQUIERDO %d",getDisMaxIz());
            escribir(cadena,linea++);
            break;
        case 2:
            sprintf(cadena, "CENTRAL %d", getDisMaxC());
            escribir(cadena,linea++);
            sprintf(cadena, "DERECHO %d",getDisMaxDr());
            escribir(cadena,linea++);
            sprintf(cadena, "IZQUIERDO %d",getDisMaxIz());
            halLcdPrintLine(cadena,linea++, INVERT_TEXT);
            break;
        default:
            sprintf(cadena, "CENTRAL %d", getDisMaxC());
            escribir(cadena,linea++);
            sprintf(cadena, "DERECHO %d",getDisMaxDr());
            escribir(cadena,linea++);
            sprintf(cadena, "IZQUIERDO %d",getDisMaxIz());
            halLcdPrintLine(cadena,linea++, INVERT_TEXT);
            break;
    }
}

void menuCreditos(){
    uint8_t i = 2;
    for (i = 2; i < 10; i++){
        borrar(i);
    }
    linea = 2;

    sprintf(cadena, "FrungiBot");
    halLcdPrintLine(cadena,linea++, INVERT_TEXT);
    sprintf(cadena, "  - Roger RC");
    escribir(cadena,linea++);
    sprintf(cadena, "  - Juan CF");
    escribir(cadena,linea++);
}

void accionesMenu(uint8_t selected){
    struct RxReturn estructura;
    uint8_t snsDerecho = 0, snsCentral = 0, snsIzquierdo = 0;
    switch(menu){
    case 0:
        switch(selected){
            case 0:
                menu = 1;
                selectedSubMenus = 0;
                menuModo();
                break;
            case 1:
                menu = 2;
                selectedSubMenus = 0;
                menuPared();
                break;
            case 2:
                menu = 3;
                selectedSubMenus = 0;
                menuSensores();
                break;
            case 3:
                menu = 4;
                menuCreditos();
                break;
            case 4:
                if(!mov){
                    run();
                    mov = true;
                }
                break;

        }
        break;
    case 1:
        switch(selectedSubMenus){
            case 0:
                setvelocidad(!getvelocidad());
                break;
            case 1:
                setDiscoMode(!getDiscoMode());
                break;
            case 2:
                setIntermitentes(!getIntermitentes());
                break;
        }
        break;
    case 2:
        switch(selectedSubMenus){
            case 0:
                setPared(5);
                break;
            case 1:
                setPared(PARED_I);
                break;
            case 2:
                setPared(PARED_D);
                break;
        }
        break;
    case 3:
         estructura = llegirSensors();
         snsIzquierdo = estructura.StatusPacket[5];
         snsCentral = estructura.StatusPacket[6];
         snsDerecho = estructura.StatusPacket[7];
            switch(selectedSubMenus){
                case 0:
                    setDisMaxC( snsCentral);
                    break;
                case 1:
                    setDisMaxDr( snsDerecho);
                    break;
                case 2:
                    setDisMaxIz( snsIzquierdo);
                    break;
            }
            break;
    }
}


/**************************************************************************
 * DELAY - A CONFIGURAR POR EL ALUMNO - con bucle while
 *
 * Datos de entrada: Tiempo de retraso. 1 segundo equivale a un retraso de 1000000 (aprox)
 *
 * Sin datos de salida
 *
 **************************************************************************/
void main(void)
{

  	WDTCTL = WDTPW+WDTHOLD;       	// Paramos el watchdog timer

    //Inicializaciones:
    init_ucs_24MHz();       //Ajustes del clock (Unified Clock System)
    init_botons();         //Configuramos botones y leds
    init_interrupciones();  //Configurar y activar las interrupciones de los botones
    init_timers();       //inicializamos los timers
    init_LCD();			    // Inicializamos la pantalla
    init_UART();        //inicializamos la UART
    init_variables();
    halLcdPrintLine(saludo,linea, INVERT_TEXT); //escribimos saludo en la primera linea
  	linea++;
  	linea++; //Aumentamos el valor de linea y con ello pasamos a la linea siguiente
  	moure_endavant();    //decimos que los motores no paren el movimiento si no se le ordena
  	struct RxReturn estructura;   //Estructura para valorar los valores que retorna los sensores de proximidad
  	uint8_t selected = 0;

  	setPared(5);
  	imprimirMenu(selected);
  	do{

  	  /************************************** SENSORES *************************************************/
  	  estructura = llegirSensors();

      uint8_t id = estructura.StatusPacket[2],
              snsIzquierdo = estructura.StatusPacket[5],
              snsCentral = estructura.StatusPacket[6],
              snsDerecho = estructura.StatusPacket[7];

      if(id == 100){
        if(snsIzquierdo >= 100 || snsCentral >= 100 || snsDerecho >= 100)
            parar_robot();
      }
      bool timeout;
      Reset_Timeout();
      while (0) //Se_ha_recibido_Byte())
      {
          timeout=TimeOut(10000); // tiempo en decenas de microsegundos (ara 10ms)
          if (timeout)break;//sale del while
      }
      if(mov)
          seguirPared(estructura);

      /*************************************************************************************************/
      if(menu==3){
          sprintf(cadena1,"%d %d %d     ", snsIzquierdo, snsCentral, snsDerecho);    // Guardamos en cadena la siguiente frase: estado "valor del estado"
          escribir(cadena1,7);
      }
      else if(menu==2){
          sprintf(cadena, "Pared %d", estado_pared);
          escribir(cadena,7);
      }
  	    if (estado_anterior != estado){			// Dependiendo del valor del estado se encender� un LED u otro.{
            sprintf(cadena," Estado %d", estado);    // Guardamos en cadena la siguiente frase: estado "valor del estado"
            //escribir(cadena,linea);          // Escribimos la cadena al LCD
      // Escribimos la cadena al LCD
            estado_anterior = estado;          // Actualizamos el valor de estado_anterior, para que no est� siempre escribiendo.


            /**********************************************************+
                A RELLENAR POR EL ALUMNO BLOQUE switch ... case
            Para gestionar las acciones:
            Boton S1, estado = 1
            Boton S2, estado = 2
            Joystick left, estado = 3
            Joystick right, estado = 4
            Joystick up, estado = 5
            Joystick down, estado = 6
            Joystick center, estado = 7
            ***********************************************************/


            //Inicialmente el robot se mueve en modo slow por defecto

            switch(estado){
                case Button_S1:
                    accionesMenu(selected);
                    //mover_fast(sentido);  //Aumenta la velocidad de movimiento
                    //vel = true;
                    break;

                case Button_S2:
                    menu = 0;
                    selected = 0;
                    //mover_slow(sentido);  //Modo por defecto y hace que el robot se mueva a una velocidad adecuada
                    //vel = false;
                    break;

                case Jstick_Up:
                    //sentido = true;       //Sentido hacia adelante
                    //mover_slow(sentido);  //mueve el robot en modo SLOW hacia adelante
                    if(selected > 0)
                        selected --;
                    if (selectedSubMenus > 0)
                        selectedSubMenus --;
                    break;
                case Jstick_Down:
                    //sentido = false;      //Sentido hacia atrás
                    //mover_slow(sentido);  //mueve el robot en modo SLOW hacia atrás
                    //mov = true;
                    //setPared(5);
                    if (selected < 4)
                        selected ++;
                    if (selectedSubMenus < 2)
                        selectedSubMenus ++;
                    break;

                case Jstick_Left:
                    //girar(false);         //gira en modo SLOW hacia la izquierda indefinidamente
                    break;

                case Jstick_Right:
                    //girar(true);          //gira en modo FAST hacia la derecha indefinidamente
                    break;

                case Jstick_Center:
                    parar_robot();       //Para los motores 2 y 3
                    mov = false;
                    apagarLeds();
                    break;
                }
            seleccionaMenu(selected);
            }


	}while(1); //Condicion para que el bucle sea infinito
}


/**************************************************************************
 * RUTINAS DE GESTION DE LOS BOTONES:
 * Mediante estas rutinas, se detectar� qu� bot�n se ha pulsado
 *
 * Sin Datos de entrada
 *
 * Sin datos de salida
 *
 * Actualizar el valor de la variable global estado
 *
 **************************************************************************/

//ISR para las interrupciones del puerto 3:
void PORT3_IRQHandler(void){//interrupcion del pulsador S2
	uint8_t flag = P3IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P3IE &= 0xDF;  //interrupciones del boton S2 en port 3 desactivadas
    estado_anterior=0;
    switch(flag){
    case 0x0C: //pin 5
        estado = Button_S2;
        break;
    }

    P3IE |= 0x20;   //interrupciones S2 en port 3 reactivadas
}

//ISR para las interrupciones del puerto 4:
void PORT4_IRQHandler(void){  //interrupci�n de los botones. Actualiza el valor de la variable global estado.
	uint8_t flag = P4IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
	P4IE &= 0x5D; 	//interrupciones Joystick en port 4 desactivadas
	estado_anterior=0;

	switch(flag){
	        case 0x04: //pin 1
	            estado = Jstick_Center; //center
	            break;
	        case 0x0C: //pin 5
	            estado = Jstick_Right; //right
	            break;
	        case 0x10: //pin 7
	            estado = Jstick_Left; //Left
	            break;
	        }

	P4IE |= 0xA2; 	//interrupciones Joystick en port 4 reactivadas
}

//ISR para las interrupciones del puerto 5:
void PORT5_IRQHandler(void){  //interrupci�n de los botones. Actualiza el valor de la variable global estado.
	uint8_t flag = P5IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
	P5IE &= 0xCD;   //interrupciones Joystick y S1 en port 5 desactivadas
	estado_anterior=0;
	switch(flag){
	        case 0x04: //pin 1
	            estado = Button_S1; //center
	            break;
	        case 0x0A:// pin 4
	            estado = Jstick_Up;
	            break;
	        case 0x0C: //pin 5
	            estado = Jstick_Down; //right
	            break;
	        }
    /***********************************************
     * HASTA AQUI BLOQUE CASE
     ***********************************************/

    P5IE |= 0x32;   //interrupciones Joystick y S1 en port 5 reactivadas
}
