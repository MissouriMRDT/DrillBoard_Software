#include "RoveSci.h"

//////////////////////////////////////
//          Pin Assignments         //
//////////////////////////////////////

#define PIN_T1          		11 // DS18B20
#define PIN_M1          		A2 // Grove
#define PIN_M3          		A3 // FC28

#define IN_A            		5 // Input A
#define IN_B            		6 // Input B
#define CS              		A0 // Current sense
#define PWM             		7 // PWM
#define CS_DIS          		12 // Current Sense Disable
#define EN_A            		8 // Two pin error detection
#define EN_B            		9 // Two pin error detection 

/////////////////////////////////////
// Received Messages from RoveComm //
/////////////////////////////////////

#define DS18B20_ENABLE  		0x00000001 // 1
#define GROVE_ENABLE    		0x00001001 // 9
#define FC28_ENABLE     		0x00001101 // 13

#define DRILL_FWD       		0x00000010 // 2
#define DRILL_STOP      		0x00000011 // 3
#define DRILL_REV       		0x00000100 // 4

//////////////////////////////
//  Drill Command Functions //
//////////////////////////////

void drillForward();
void drillReverse();
void drillStop();

/////////////////////////////////////
// Drew Bischoff Gripper functions //
/////////////////////////////////////

#define CLOCKWISE 					0
#define COUNTERCLOCKWISE 		1
#define DRILL_SPEED   			50

// Current sensing constants
#define K_FACTOR 						770.0
#define SENSE_RESISTOR 			223.0
#define MAX_RESOLUTION 			1023.0 //previously 4096
#define MAX_VOLTAGE 				3.3 //previously 3.3
#define CURRENT_SENSE_SCALE ((MAX_VOLTAGE*K_FACTOR/MAX_RESOLUTION)/SENSE_RESISTOR)


int presentPWM = 0;
boolean stoppedMotor = true;
int presentDirection = CLOCKWISE;

void setMotorSpeed(int spd);
void setDirection(int dir);
void enableMotor();
void stopRotation();
void rotateMotor(int dir, int spd);
void rampPWM(int targetPWM);

/////////////////////////
//  Start board code ! //
///////////////////////// 

void setup() {
  Serial.begin(9600); // may change for drill

  pinMode(EN_A, INPUT); 
  pinMode(EN_B, INPUT); 
  pinMode(IN_A, OUTPUT); 
  pinMode(IN_B, OUTPUT); 
  pinMode(CS, INPUT); 
  //pinMode(CS_DIS, OUTPUT); 
  pinMode(PWM, OUTPUT); 
  
  //disable motors and enable current sense at start
  digitalWrite(IN_A,0);
  digitalWrite(IN_B,0);
  //digitalWrite(CS_DIS,0);//CS enabled when CS_DIS=0
  
  /* 
   * delay(1000);
   * rotateMotor(CLOCKWISE,20);
   * delay(300);
   * rotateMotor(COUNTERCLOCKWISE,20);
   * delay(300);
   * stopRotation();
   */
}

void loop() {
  // Get command from science board
  while(!Serial.available());
  byte drill_cmd = Serial.read();
  
  switch(drill_cmd){
    // Read sensors
    case GROVE_ENABLE:
      Serial.write(roveSci_Grove_ReadHumid(PIN_M1));
      break;
    case FC28_ENABLE:
      Serial.write(roveSci_FC28_ReadHumid(PIN_M3));
      break;
    case DS18B20_ENABLE:
      Serial.write(static_cast<int>(10 * roveSci_DS18B20_ReadTemp(PIN_T1)));
			// Scale back down after being received. 
      break;
    
    // Control drill
    case DRILL_FWD:
      drillForward();
      break;
    case DRILL_REV:
      drillReverse();
      break;
    case DRILL_STOP:
      drillStop();
      break;
  }
}

void drillForward() {
  rotateMotor(CLOCKWISE, DRILL_SPEED);
  return;
}

void drillReverse() {
  rotateMotor(COUNTERCLOCKWISE, DRILL_SPEED);
  return;
}

void drillStop() {
  stopRotation();
  return;
}

////////////////////////////////////////////////////////
//     DREW BISCHOFF GRIPPER CODE: DO NOT CHANGE      //
////////////////////////////////////////////////////////

// 0 < spd < 100
void setMotorSpeed(int spd){
  spd*=255;
  spd/=100;
  analogWrite(PWM,spd);//takes 0-255 as input
 // Serial.println(spd);//DEBUG to make sure ramp works
}

void setDirection(int dir){
  presentDirection = dir;

  if(dir==CLOCKWISE){
    digitalWrite(IN_A,1);
    digitalWrite(IN_B,0);
  }
  else{
    digitalWrite(IN_A,0);
    digitalWrite(IN_B,1);
  }
}

void enableMotor(){
  //digitalWrite(IN_A,1);
  //digitalWrite(IN_B,0);
  stoppedMotor=false;
}

void stopRotation(){
  digitalWrite(IN_A,0);
  digitalWrite(IN_B,0);
  stoppedMotor=true;   
  presentPWM=0;
}

int currentTolerance = 5; //the highest value for analog read that will be viewed as overcurrent.
void rotateMotor(int dir, int spd){
  
  if(presentDirection!=dir){
    stopRotation();
  }
  if(stoppedMotor){
      delay(10);//enough time to come to a complete stop
  } 
  setDirection(dir);
  enableMotor();
  rampPWM(spd);  
}

int incrementAmount=5;
int delayAmount=10;
void rampPWM(int targetPWM){
  while(true){
    if(presentPWM>targetPWM){
      presentPWM-=incrementAmount;
    }
    else{
      presentPWM+=incrementAmount;
    }    
    if(presentPWM < (targetPWM+incrementAmount/2.0) && presentPWM > (targetPWM-incrementAmount/2.0)){
      presentPWM=targetPWM;
      setMotorSpeed(presentPWM);
      break;
    }
    setMotorSpeed(presentPWM);
    delay(delayAmount);       
  } 
}
