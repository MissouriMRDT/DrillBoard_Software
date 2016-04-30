#include "RoveSci.h"


// MUTABLE VARIABLES
const int PIN_T1 = 11; // DS18B20
const int PIN_M1 = A2; // Grove
const int PIN_M3 = A3; // FC28

const int PIN_D_INA = ; // 
const int PIN_D_INB = ; // 
const int PIN_D_CS = ; // Current sense
const int PIN_D_PWM = ; // 
const int PIN_D_CS_DIS = ; // Current Sense Disable
const int PIN_D_ENA = ; // Two pin error detection
const int PIN_D_ENB = ; // Two pin error detection 

const byte DS18B20_ENABLE = 1;
const byte GROVE_ENABLE = 9;
const byte FC28_ENABLE = 13;

const byte DRILL_FWD = 2;
const byte DRILL_STOP = 3;

const int DELTA_SPEED = 10;
const int MAX_SPEED = 85;
const int STEP_DELAY = 500; // milliseconds

void drillEnable();
void drillDisable();


void setup() {
  Serial.begin(9600); // may change for drill
  
}

void loop() {
  // Get command from science board
  while(!Serial.available());
  byte drill_cmd = Serial.read();
  float temp_read = 0;
  byte * b;
  
  switch(drill_cmd){
    // Read sensors
    case GROVE_ENABLE:
      Serial.write(static_cast<int>(roveSci_Grove_ReadHumid(PIN_M1)));
      break;
    case FC28_ENABLE:
      Serial.write(static_cast<int>(roveSci_FC28_ReadHumid(PIN_M3)));
      break;
    case DS18B20_ENABLE:
      temp_read = roveSci_DS18B20_ReadTemp(PIN_T1);
      b = (byte *) &temp_read; // write float, weird serial spec
      Serial.write(b[3]);
      Serial.write(b[2]);
      Serial.write(b[1]);
      Serial.write(b[0]);  
      break;
    
    // Control drill
    case DRILL_FWD:
      drillEnable();
      break;
    case DRILL_STOP:
      drillDisable();
      break;
  }
}

void drillEnable()
{

}

void drillDisable()
{

}
