#include "RoveSci.h"

//////////////////////////////////////
//          Pin Assignments         //
//////////////////////////////////////

#define PIN_T1          		11 // PB3 (MOSI)
#define PIN_T2              10 // PB2 (SS')
#define PIN_T3              4  // PD4
#define PIN_T4              3  // PD3 (INT1)

#define PIN_M1          		A2 // PC2
#define PIN_M2              A1 // PC1
#define PIN_M3          		A3 // PC3
#define PIN_M4              A4 // PC4 (SDA)

#define IN_A            		5 // Input A
#define IN_B            		6 // Input B
#define CS              		A0 // Current sense
#define PWM             		7 // PWM
#define CS_DIS          		12 // Current Sense Disable
#define EN_A            		8 // Two pin error detection
#define EN_B            		9 // Two pin error detection 

//////////////////////////////////////////
// Received Messages from Science Board //
//////////////////////////////////////////

#define DRILL_STOP          0
#define DRILL_FWD           1
#define DRILL_REV           2
#define T1_ON               3
#define T2_ON               4
#define T3_ON               5
#define T4_ON               6
#define M1_ON               7
#define M2_ON               8
#define M3_ON               9
#define M4_ON               10

#define TEMP_SCALE          10

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
#define MAX_VOLTAGE 				3.3
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
byte board_cmd;
void loop() {
  // Get command from science board
  // while(!Serial.available());
  // byte board_cmd = Serial.read();
  board_cmd = (board_cmd++)%3 + 2;
  
  switch(board_cmd){
    // Read sensors
    case T1_ON:
      Serial.write(static_cast<int>(TEMP_SCALE * roveSci_DS18B20_ReadTemp(PIN_T1)));
      break;
    case T2_ON:
      Serial.write(static_cast<int>(TEMP_SCALE * roveSci_DS18B20_ReadTemp(PIN_T1)));
      break;
    case T3_ON:
      Serial.write(static_cast<int>(TEMP_SCALE * roveSci_DS18B20_ReadTemp(PIN_T1)));
      break;
    case T4_ON:
      Serial.write(static_cast<int>(TEMP_SCALE * roveSci_DS18B20_ReadTemp(PIN_T4)));
      break;
    case M1_ON:
      roveSci_XD28_ReadHumidity(PIN_M1);
      break;
    case M2_ON:
      roveSci_XD28_ReadHumidity(PIN_M2);
      break;
    case M3_ON:
      roveSci_XD28_ReadHumidity(PIN_M2);
      break;
    case M4_ON:
      roveSci_XD28_ReadHumidity(PIN_M2);
      break;
    
    // Control drill
    case DRILL_STOP:
      drillStop();
      break;
    case DRILL_FWD:
      drillForward();
      break;
    case DRILL_REV:
      drillReverse();
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
