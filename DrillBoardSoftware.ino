//Arduino
#include "DualVNH5019MotorShield.h"
#include "EasyTransfer.h"
#include "OneWire.h"

//RoveWare
#include "RoveSense.h"

#define SOFT_WATCHDOG_MILLIS    500

#define DRILL_STOP              0
#define DRILL_FORWARD           1
#define DRILL_REVERSE           2 

#define BRAKE_5019_INDUCTIVE    400
#define SPEED_5019_MAX_REVERSE -400
#define SPEED_5019_MAX_FORWARD  400

#define BRAKE_DELAY             250
#define RAMP_DELAY              2

#define DRILL_CURRENT_FAULT     -1.0

 // Default VNH5019 motor shield pinmap
 // _PWM1 = 9
 // _INA1 = 2;
 // _INB1 = 4;
 // _EN1DIAG1 = 6;
 // _CS1 = A0; 
 
DualVNH5019MotorShield DrillMotor;

uint8_t  last_drill_command = 0;
uint32_t last_watch_clear_millis = 0;
 
////////////////////////////////////////////////////////////////////////////////////////////////
#define PIN_T1              11 // PB3 (MOSI)
#define PIN_T2              10 // PB2 (SS')
#define PIN_T3              4  // PD4
#define PIN_T4              3  // PD3 (INT1)
#define PIN_M1              A2 // PC2
#define PIN_M2              A1 // PC1
#define PIN_M3              A3 // PC3
#define PIN_M4              A4 // PC4 (SDA)
#define PIN_M4_D3           A5  //PC5       
#define PIN_M4_D4           2   //PC4

//recieve sensor data to ScienceBoard
struct serial_rx 
{
  uint16_t command;
};

//send sensor data to ScienceBoard
struct serial_tx
{
  float t1_data;
  float t2_data;
  float t3_data;
  float t4_data;
  float m1_data;
  float m2_data;
  float m3_data;
  float m4_data;
  float drill_current;
};

serial_rx commands_RX;
serial_tx telem_TX;

EasyTransfer FromScienceBoard; 
EasyTransfer ToScienceBoard;

////////////////////////////////////////////////////////////////////////////////////////////////
void setup() 
{
  Serial.begin(9600);
  
  FromScienceBoard.begin(details(commands_RX), &Serial);
  ToScienceBoard.begin(  details(telem_TX),    &Serial); 
  
  DrillMotor.init();
  
  commands_RX.command = 0;

  last_watch_clear_millis = millis();
}//end if  

////////////////////////////////////////////////////////////////////////////////////////////////
void loop() 
{  
  if(FromScienceBoard.receiveData())
  {    
    switch(commands_RX.command) 
    {  
      case DRILL_STOP:
      
        DrillMotor.setM1Brake(BRAKE_5019_INDUCTIVE);
        stopIfFault();
        delay(BRAKE_DELAY);
        break;
        
      case DRILL_FORWARD:

        if(commands_RX.command != last_drill_command && last_drill_command != DRILL_STOP)
        {  
          DrillMotor.setM1Brake(BRAKE_5019_INDUCTIVE);
          stopIfFault();
          delay(BRAKE_DELAY);
        }//end if
        
        rampDrillSpeed(0, SPEED_5019_MAX_FORWARD);
        delay(RAMP_DELAY);
        break;
              
     case DRILL_REVERSE:

        if(commands_RX.command != last_drill_command && last_drill_command != DRILL_STOP)
        {  
          DrillMotor.setM1Brake(BRAKE_5019_INDUCTIVE);
          stopIfFault();
          delay(BRAKE_DELAY);
        }//end if
        
        rampDrillSpeed(0, SPEED_5019_MAX_REVERSE);
        delay(RAMP_DELAY);
        break;
        
     default:
       break;
    }//end switch 
  }//end if
  
  if( (millis() - last_watch_clear_millis ) > SOFT_WATCHDOG_MILLIS)
  {
    DrillMotor.setM1Brake(BRAKE_5019_INDUCTIVE);
    stopIfFault();
    delay(BRAKE_DELAY);
  }//end if 

  telem_TX.t1_data = RoveSense_TempDS18B20_read(PIN_T1);
  telem_TX.t2_data = RoveSense_TempDS18B20_read(PIN_T2);
  telem_TX.t3_data = RoveSense_TempDS18B20_read(PIN_T3);
  telem_TX.t4_data = RoveSense_TempDS18B20_read(PIN_T4);
  
  telem_TX.m1_data = RoveSense_HumidityXD28_read(PIN_M1);
  telem_TX.m2_data = RoveSense_HumidityXD28_read(PIN_M2);
  telem_TX.m3_data = 0; // Todo
  telem_TX.m4_data = RoveSense_HumidityCustom_read(PIN_M4, PIN_M4_D3, PIN_M4_D4);
  
  telem_TX.drill_current = 0; // Todo 

  ToScienceBoard.sendData();  

}//end loop 

////////////////////////////////////////////////////////////////////////////////////////////////
void stopIfFault()
{
  if (DrillMotor.getM1Fault())
  {
    DrillMotor.setM1Brake(BRAKE_5019_INDUCTIVE);
    while(1)
    {
      telem_TX.drill_current = DRILL_CURRENT_FAULT;
      ToScienceBoard.sendData();
    }//end while
  }//end if
}//end fnctn

/////////////////////////////////////////////////////////////////////////////////////////////////
void rampDrillSpeed(int start_value, int end_value) 
{ 
  if(start_value < end_value)
  {
    for(int i = start_value; i < end_value; i++)
    { 
      DrillMotor.setM1Speed(i);
      stopIfFault();     
    }//end while   
  }else{
    
    for(int i = start_value; i > end_value; i--)
    { 
      DrillMotor.setM1Speed(i);
      stopIfFault();      
    }//end while
  }//end if 
}//end fnctn
