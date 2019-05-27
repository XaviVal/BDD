
#include <moure_robot.h>

void setPared(uint8_t pared){
    estado_pared = pared;
}


void seguirPared(struct RxReturn estructura) {

    uint8_t sensorEsq = estructura.StatusPacket[5],
            sensorCentre = estructura.StatusPacket[6],
            sensorDreta = estructura.StatusPacket[7];

    if(sensorCentre >= getDisMaxC()){
        haTrobatPared = true;
        if(sensorDreta <= 0){
            if(estado_pared == 5)
                estado_pared = PARED_I;
        }else if(sensorEsq <= 0){
            if(estado_pared == 5)
                estado_pared = PARED_D;
        }else
            parar_robot();
    }
    if(!haTrobatPared){
        moure_endavant();
        return;
    }

    switch(estado_pared){
        case PARED_I:
            if(sensorCentre >= getDisMaxC() || sensorEsq >= getDisMaxIz()){
                moure_esq();
                // if(getIntermitentes()) encenderLedsMotor(true);
                // else apagarLedsMotor();
            }
            else if (sensorCentre <= disMinC && sensorEsq <= disMinIz){
                moure_dreta();
                // if(getIntermitentes()) encenderLedsMotor(false);
                // else apagarLedsMotor();
            }
            else{
                moure_endavant();
            }
            break;
        case PARED_D:
            if (sensorCentre >= disMaxC || sensorDreta >= disMaxDr){
                moure_dreta();
                // if(getIntermitentes()) encenderLedsMotor(false);
                // else apagarLedsMotor();

            }else if (sensorCentre <= disMinC && sensorDreta <= disMinDr){
                moure_esq();
                // if(getIntermitentes()) encenderLedsMotor(true);
                // else apagarLedsMotor();
            }
            else{
                moure_endavant();
            }
            break;
        default:
            moure_endavant();
            break;
    }
}

bool getvelocidad(){
    return velocidad2;
}
bool getDiscoMode(){
    return discoMode;
}
bool getIntermitentes(){
    return intermitentes;
}
void setvelocidad(bool vel){
    velocidad2 = vel;
}
void setDiscoMode(bool mode){
    discoMode = mode;
}
void setIntermitentes(bool mode){
    intermitentes = mode;
}

uint8_t getDisMaxC(){
    return disMaxC;
}
uint8_t getDisMaxIz(){
    return disMaxIz;
}
uint8_t getDisMaxDr(){
    return disMaxDr;
}
void setDisMaxC(uint8_t sns){
    disMaxC = sns;
    if(disMaxC - disMinC >= disMinC) disMinC = disMaxC - disMinC;
}
void setDisMaxIz(uint8_t sns){
    disMaxIz = sns;
    if(disMaxIz - disMinIz >= disMinIz) disMinIz = disMaxIz - disMinIz;
}
void setDisMaxDr(uint8_t sns){
    disMaxDr = sns;
    if(disMaxDr - disMinDr >= disMinDr) disMinDr = disMaxDr - disMinDr;
}

void modoDisco(void){
    P2OUT |= 0x50;  //Encendemos el led R y G
    P5OUT |= 0x40;  //Encendemos el led B
    P7OUT = 127;
    musica();
}

void config_P7_LEDS (void)
{
    P7OUT &= ~0xFF;
}

void ApagarRGB_Launchpad(){
    P2OUT &= ~0x50; //Primero apago los Leds del P2
    P5OUT &= ~0x40; //apago los leds del P5
}

void apagarLeds(void){
    ApagarRGB_Launchpad(); //Apagamos el led RGB
    config_P7_LEDS();
    // apagarLedsMotor();
}

void init_variables(){
    velocidad2 = true;
    discoMode = true;
    intermitentes = true;
    haTrobatPared = false;
    disMaxC = 100;
    disMaxIz = 120;
    disMaxDr = 120;
    disMinC = 50;
    disMinIz = 50;
    disMinDr = 50;
}

void run(){
    if(getDiscoMode()) modoDisco();
    else apagarLeds();
    moure_endavant();
    haTrobatPared = false;

}


