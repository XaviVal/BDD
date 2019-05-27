#ifndef MOURE_ROBOT_H_
#define MOURE_ROBOT_H_

#include "instruccions_a_robot.h"
#include "instruccions_a_sensor.h"


#define PARED_I 0
#define PARED_C 1
#define PARED_D 2

uint16_t tempsA2;

bool velocidad2;
bool discoMode;
bool intermitentes;
bool haTrobatPared;


uint8_t estado_pared;

uint8_t disMaxC;
uint8_t disMaxIz;
uint8_t disMaxDr;
uint8_t disMinC;
uint8_t disMinIz;
uint8_t disMinDr;


void seguirPared(struct RxReturn estructura);
void setPared(uint8_t pared);
bool getVelocidad();
bool getDiscoMode();
bool getIntermitentes();
void setVelocidad(bool vel);
void setDiscoMode(bool mode);
void setIntermitentes(bool mode);

uint8_t getDisMaxC();
uint8_t getDisMaxIz();
uint8_t getDisMaxDr();
void setDisMaxC(uint8_t sns);
void setDisMaxIz(uint8_t sns);
void setDisMaxDr(uint8_t sns);
void apagarLeds(void);
void run(void);
void init_variables(void);


#endif
