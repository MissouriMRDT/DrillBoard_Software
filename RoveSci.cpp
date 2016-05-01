#include <OneWire.h>
#include "Arduino.h"
/* // Tiva only version ds18b20
float roveSci_DS18B20_ReadTemp(int data_pin) {
  int DS18B20_Pin = data_pin;
  DS18B20 ds(DS18B20_Pin); // split into DS18B20_Begin? removes param
  return float(ds.GetData());
} */

int roveSci_XD28_ReadHumidity(int data_pin) {
	int sensorValue = 0;
	return sensorValue;
}

// Arduino version
float roveSci_DS18B20_ReadTemp(int data_pin) {
	OneWire ds(data_pin);

	byte data[12];
	byte addr[8];
  
	ds.reset();
	// ds.select(addr);
	ds.write(0xCC); // Broadcasting to all devices. Should find singular address
	ds.write(0x44,1); // start conversion, with parasite power on at the end
  
	byte present = ds.reset();
	//  ds.select(addr);
	ds.write(0xCC); // Broadcasting to all devices. Should find singular address
	ds.write(0xBE); // Read Scratchpad
  
	for (int i = 0; i < 9; i++) { // we need 9 bytes
		data[i] = ds.read();
	}
  
	ds.reset_search();
  
	byte MSB = data[1];
	byte LSB = data[0];
  
	float tempRead = ((MSB << 8) | LSB); // using two's complement 
	float TemperatureSum = tempRead / 16;
	return TemperatureSum;
}
/* Tiva values: calibrate for Arduino */
int roveSci_Grove_ReadHumid(int data_pin) {
  int sensorValue = analogRead(data_pin) * 100 / 4095; // 1023 for arduino?
  return  sensorValue;
} 

/* Tiva values: calibrate for Arduino 
	Not being used, extreme low priority */
int roveSci_FC28_ReadHumid(int data_pin) {
  int percentage = -(analogRead(data_pin) - 4095) * 100 / 1850;
  if (percentage >= 100)
  	return 100;
  else
    return percentage;
}

/* SHT10 stuff
 //not being used anyway. No need to have Arduino version afaik 
float roveSci_SHT10_ReadTemp(int data_pin, int clock_pin) {
  SHT10 sht(data_pin, clock_pin); // same deal as DS18B20
  return sht.readTemperature(); 
}
// not being used anyway. No need to have Arduino version afaik 
	Added value if it can be done but not important 
float roveSci_SHT10_ReadHumid(int data_pin, int clock_pin) {
  SHT10 sht(data_pin, clock_pin);
  return sht.readHumidity(); 
} */
