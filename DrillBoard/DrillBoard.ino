#include "RoveWare.h"

////////////////////////////////////////
// Drill pins drive Motor 1 on header X7

const uint8_t DRILL_INA_PIN  = PH_0;
const uint8_t DRILL_INB_PIN  = PP_5;
const uint8_t DRILL_PWM_PIN = PG_1;

/////////////////////////////////////////////////////////////////
// Geneva pins drive Motor 2 on header X7

const uint8_t GENEVA_INA_PIN          = PH_1;
const uint8_t GENEVA_INB_PIN          = PA_7;
const uint8_t GENEVA_PWM_PIN          = PK_4;

const uint8_t GENEVA_LIMIT_SWITCH_PIN   = PK_1; // Uses LS 1 on X7
const uint8_t CAROUSEL_LIMIT_SWITCH_PIN = PK_0; // Uses LS 2 on X7

/////////////////////////////////////////////////////////////////////////////////
// Lead Screw pins drive Motor 1 on header X9

const uint8_t LEADSCREW_INA_PIN           = PL_0;
const uint8_t LEADSCREW_INB_PIN           = PH_2;
const uint8_t LEADSCREW_PWM_PIN           = PF_1;

const uint8_t LEADSCREW_LIMIT_SWITCH_1_PIN  = PE_3; //Uses LS 1 on X9; Bottom LS
const uint8_t LEADSCREW_LIMIT_SWITCH_2_PIN  = PE_2; //Uses LS 2 on X9; Deploy LS
const uint8_t LEADSCREW_LIMIT_SWITCH_3_PIN  = PE_1; //Uses LS 3 on X9; Reload LS
const uint8_t LEADSCREW_LIMIT_SWITCH_4_PIN  = PE_0; //Uses LS 4 on X9; Empty LS
const uint8_t LEADSCREW_LIMIT_SWITCH_5_PIN  = PB_5; //Uses LS 3 on X7; Top LS

#define LEADSCREW_BOTTOM_LIMIT_SWITCH_PIN  LEADSCREW_LIMIT_SWITCH_1_PIN
#define LEADSCREW_DEPLOY_LIMIT_SWITCH_PIN  LEADSCREW_LIMIT_SWITCH_2_PIN
#define LEADSCREW_RELOAD_LIMIT_SWITCH_PIN  LEADSCREW_LIMIT_SWITCH_3_PIN
#define LEADSCREW_EMPTY_LIMIT_SWITCH_PIN   LEADSCREW_LIMIT_SWITCH_4_PIN
#define LEADSCREW_TOP_LIMIT_SWITCH_PIN     LEADSCREW_LIMIT_SWITCH_5_PIN

////////////////////////////////////////////
//Global Consts
const int LEADSCREW_CLOSED_LOOP_SPEED = 500;
const int GENEVA_CLSED_LOOP_SPEED    = 500;

//Global variables
uint16_t drill_speed;

int geneva_position = 1; 
uint16_t geneva_speed;
uint8_t geneva_goto_position = 0;
  /*
   * 0-Do Nothing
   * 1-Pos 1
   * 2-Pos 2
   * ...
   * 6-Pos 6
   */
uint16_t leadscrew_speed;
bool leadscrew_going_up = 1;
int leadscrew_position  = 0;
  /*
   * 1-Between Bottom and Deploy
   * 2-Between Deploy and Reload
   * 3-Between Reload and Empty
   * 4-Between Empty and Top
   */
uint8_t leadscrew_goto_position = 0;
  /*
   * 0-Do Nothing (open loop
   * 1-Bottom LS
   * 2-Deploy LS
   * 3-Empty LS
   * 4-Reload LS
   * 5-Top LS
   */

/////////////////////////////////////////
RoveCommEthernetUdp  RoveComm;

RoveWatchdog         Watchdog;

RoveVnh5019          DrillMotor;
RoveVnh5019          GenevaMotor;
RoveVnh5019          LeadScrewMotor;

void estop();

////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() 
{
  RoveComm.begin(ROVE_FIRST_OCTET, ROVE_SECOND_OCTET, ROVE_THIRD_OCTET, DRILLBOARD_FOURTH_OCTET);
  delay(1);
  
  Serial.begin(9600);
  delay(1);

  pinMode(DRILL_INA_PIN,     OUTPUT);
  pinMode(DRILL_INB_PIN,     OUTPUT);
  pinMode(DRILL_PWM_PIN,     OUTPUT);
  
  pinMode(GENEVA_INB_PIN,    OUTPUT);
  pinMode(GENEVA_INB_PIN,    OUTPUT);
  pinMode(GENEVA_INB_PIN,    OUTPUT);
  
  pinMode(LEADSCREW_INB_PIN, OUTPUT);
  pinMode(LEADSCREW_INB_PIN, OUTPUT);
  pinMode(LEADSCREW_INB_PIN, OUTPUT);

  pinMode(GENEVA_LIMIT_SWITCH_PIN,      INPUT);
  pinMode(CAROUSEL_LIMIT_SWITCH_PIN,    INPUT);

  pinMode(LEADSCREW_LIMIT_SWITCH_1_PIN, INPUT);
  pinMode(LEADSCREW_LIMIT_SWITCH_2_PIN, INPUT);
  pinMode(LEADSCREW_LIMIT_SWITCH_3_PIN, INPUT);
  pinMode(LEADSCREW_LIMIT_SWITCH_4_PIN, INPUT);
  pinMode(LEADSCREW_LIMIT_SWITCH_5_PIN, INPUT);
  
  LeadScrewMotor.begin(LEADSCREW_INA_PIN, LEADSCREW_INB_PIN, LEADSCREW_PWM_PIN);
  GenevaMotor.begin(   GENEVA_INA_PIN,    GENEVA_INB_PIN,    GENEVA_PWM_PIN   );   
  DrillMotor.begin(    DRILL_INA_PIN,     DRILL_INB_PIN,     DRILL_PWM_PIN    );  

 
  Watchdog.begin(estop, 150);
}

///////////////////////////////////////////////////////////////////

void loop()
{   
  uint16_t data_id   = 0; 
  size_t   data_size = 0; 
  uint8_t  data[2];

  RoveComm.read(&data_id, &data_size, data);

  switch(data_id)
  {
    case LEADSCREW_OPEN_LOOP:
      leadscrew_speed = *(int16_t*)(data);
      leadscrew_going_up   = (leadscrew_speed == abs(leadscrew_speed));
      
      if(digitalRead(LEADSCREW_TOP_LIMIT_SWITCH_PIN)    &&  leadscrew_going_up) break; //If you're trying to go up   when the top    limit switch is engaged, break
      if(digitalRead(LEADSCREW_BOTTOM_LIMIT_SWITCH_PIN) && !leadscrew_going_up) break; //If you're trying to go down when the bottom limit switch is engaged, break
      
      LeadScrewMotor.drive(leadscrew_speed); 
      leadscrew_goto_position = 0;      
      Watchdog.clear();
      break;

    case GENEVA_OPEN_LOOP:
      geneva_speed = *(int16_t*)(data); 
      GenevaMotor.drive(geneva_speed);
      geneva_goto_position = 0;       
      Watchdog.clear();
      break;
    
    case DRILL_OPEN_LOOP:
      drill_speed = *(int16_t*)(data);   
      DrillMotor.drive(geneva_speed);
      Watchdog.clear();
      break;
    
    case LEADSCREW_TO_LIMIT_SWITCH:
       leadscrew_goto_position = *(int8_t*)data;
       Watchdog.clear();
       break;
    
    case GENEVA_TO_POSITION:
      geneva_goto_position = *(int8_t*)data;
      if(geneva_goto_position == 1) geneva_position = 2; //Special Case: To set the geneva home based on LS readings instead of position memory
      Watchdog.clear();
      break;    
    default:
      break;
  }

  ///////////////////////////////
  //    LeadScrew Functions    //
  ///////////////////////////////

  //Limit Switch Stop////////////////////////////////////////////////////////////////////////////////////
  if(digitalRead(LEADSCREW_TOP_LIMIT_SWITCH_PIN)    &&  leadscrew_going_up) LeadScrewMotor.brake(0);
  if(digitalRead(LEADSCREW_BOTTOM_LIMIT_SWITCH_PIN) && !leadscrew_going_up) LeadScrewMotor.brake(0);

  //Track Drill Position////////////////////////////////////////////////////////////////////
  if(digitalRead(LEADSCREW_LIMIT_SWITCH_4_PIN) &&  leadscrew_going_up) leadscrew_position = 4;
  if(digitalRead(LEADSCREW_LIMIT_SWITCH_4_PIN) && !leadscrew_going_up) leadscrew_position = 3;

  if(digitalRead(LEADSCREW_LIMIT_SWITCH_3_PIN) &&  leadscrew_going_up) leadscrew_position = 3;
  if(digitalRead(LEADSCREW_LIMIT_SWITCH_3_PIN) && !leadscrew_going_up) leadscrew_position = 2;
  
  if(digitalRead(LEADSCREW_LIMIT_SWITCH_2_PIN) &&  leadscrew_going_up) leadscrew_position = 2;
  if(digitalRead(LEADSCREW_LIMIT_SWITCH_2_PIN) && !leadscrew_going_up) leadscrew_position = 1;

  //Move LeadScrew To position//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if(leadscrew_goto_position = leadscrew_position)       LeadScrewMotor.brake(0);                            //If you're at    the position, break
  else if (leadscrew_position < leadscrew_goto_position) LeadScrewMotor.drive(LEADSCREW_CLOSED_LOOP_SPEED);  //If you're below the position, move up
  else                                                   LeadScrewMotor.drive(LEADSCREW_CLOSED_LOOP_SPEED);  //else                          move down


  //////////////////////////////////////////
  //     Geneva Closed Loop Functions     //
  //////////////////////////////////////////
  if(geneva_goto_position)
  {
    //Geneva Set Home//////////////////////////////////////////////////////////////////////////////////////
    if(digitalRead(GENEVA_LIMIT_SWITCH_PIN) && digitalRead(CAROUSEL_LIMIT_SWITCH_PIN))
    {
      geneva_position = 1;
      while(digitalRead(GENEVA_LIMIT_SWITCH_PIN))
       {
         GenevaMotor.drive(GENEVA_CLSED_LOOP_SPEED);
       }
    }

    //Increment Geneva Position if limit switch is tripped
    if(digitalRead(GENEVA_LIMIT_SWITCH_PIN))
    {
       geneva_position ++;
       while(digitalRead(GENEVA_LIMIT_SWITCH_PIN))
       {
         GenevaMotor.drive(GENEVA_CLSED_LOOP_SPEED);
       }
     }

    //Geneva_position overflow//////////////////
    if(geneva_position = 7) geneva_position = 1;

    //Geneva Goto Position////////////////////////////////////////////////////////////////////////

    if(geneva_position != geneva_goto_position) GenevaMotor.drive(GENEVA_CLSED_LOOP_SPEED);
    if(geneva_position == geneva_goto_position) GenevaMotor.brake(GENEVA_CLSED_LOOP_SPEED); 
  }

  
}

//////////////////////////
void estop()
{
  DrillMotor.brake(0);  
  GenevaMotor.brake(0);
  LeadScrewMotor.brake(0);      
}

