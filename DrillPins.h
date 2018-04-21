/*
 * J7Pins.h
 *
 *  Created on: Apr 19, 2018
 *      Author: drue
 */

#ifndef DRILLPINS_H_
#define DRILLPINS_H_

#include <stdint.h>

//drill is on X7
const uint8_t DRILLMOTOR1_INAPIN = PH_0;
const uint8_t DRILLMOTOR1_INBPIN = PP_5;
const uint8_t DRILLMOTOR1_PWMPIN = PG_1;
const uint8_t DRILLMOTOR1_GEN = 2;

const uint8_t DRILLMOTOR2_INAPIN = PH_1;
const uint8_t DRILLMOTOR2_INBPIN = PA_7;
const uint8_t DRILLMOTOR2_PWMPIN = PK_4;
const uint8_t DRILLMOTOR2_GEN = 3;


#endif /* DRILLPINS_H_ */
