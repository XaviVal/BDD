/*
 * lib_instrucciones_sensor.h
 *
 *  Created on: 31 maig 2018
 *      Author: mat.aules
 */

#ifndef INSTRUCCIONS_A_SENSOR_H_
#define INSTRUCCIONS_A_SENSOR_H_

#include "comunicacio.h"

#define IR_IZQ 0x1A
#define MAX_PARAMETER_LENGTH 16

struct RxReturn llegirSensors(void);
void musica(void);

#endif /* INSTRUCCIONS_A_SENSOR_H_ */
