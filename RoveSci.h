#ifndef ROVESCI_H
#define ROVESCI_H

int roveSci_Grove_ReadHumid(int data_pin);
int roveSci_FC28_ReadHumid(int data_pin);
//float roveSci_SHT10_ReadHumid(int data_pin, int clock_pin);
//float roveSci_SHT10_ReadTemp(int data_pin, int clock_pin);
float roveSci_DS18B20_ReadTemp(int data_pin);

#endif
