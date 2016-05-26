#ifndef RoveSense_h
#define RoveSense_h

#include "OneWire.h"

#define DS_BROADCAST_ALL   0xCC
#define DS_CONVERT_TEMP    0x44
#define DS_READ_SCRATCHPAD 0xBE 
#define TEMP_SCALE         10
#define DATA_FLIP_DELAY    1000

//DS18B20 digital thermometer communicates over a 1-Wire bus and derives parasite power power directly from data line 
float RoveSense_TempDS18B20_read(int data_pin);

//4-wire LM393 Comparator  VCC: 3.3V-5V, GND, DO: digital output, AO: Analog Output Interface
float RoveSense_HumidityXD28_read(int data_pin);

//H-bridge, voltage flipping cheap two-probe soil moisture sensor without electrolysis
float RoveSense_HumidityCustom_read(int analog_read_pin, int data_flip_pin1, int data_flip_pin2);

#endif //RoveSense_h
