/*
 * J9Pins.h
 *
 *  Created on: Apr 19, 2018
 *      Author: drue
 */

#ifndef SCREWPINS_H_
#define SCREWPINS_H_

#include <stdint.h>

//screw on X9
const uint8_t SCREWMOTOR1_INAPIN = PL_0;
const uint8_t SCREWMOTOR1_INBPIN = PH_2;
const uint8_t SCREWMOTOR1_PWMPIN = PF_1;
const uint8_t SCREWMOTOR1_GEN = 0;

const uint8_t SCREWMOTOR2_INAPIN = PL_1;
const uint8_t SCREWMOTOR2_INBPIN = PH_3;
const uint8_t SCREWMOTOR2_PWMPIN = PF_2;
const uint8_t SCREWMOTOR2_GEN = 1;

const uint8_t ENCODERPIN = PD_0;
const uint8_t ENCODERGEN = 0;

const uint8_t SCREW_LIM_UP_PIN = PE_3;
const uint8_t SCREW_LIM_DOWN_PIN = PE_2;

#endif /* SCREWPINS_H_ */
