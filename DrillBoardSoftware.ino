#include "RoveWare.h"

/////////////////////////////////////////////
// Drill pins drive Motor 1 on header X7

const uint8_t DRILL_MOTOR1_INA_PIN    = PH_0;
const uint8_t DRILL_MOTOR1_INB_PIN    = PP_5;
const uint8_t DRILL_MOTOR1_PWM_PIN    = PG_1;

/////////////////////////////////////////////////
// Geneva pins drive Motor 2 on header X7

const uint8_t GENEVA_MOTOR2_INA_PIN       = PH_1;
const uint8_t GENEVA_MOTOR2_INB_PIN       = PA_7;
const uint8_t GENEVA_MOTOR2_PWM_PIN       = PK_4;

const uint8_t GENEVA_LIMIT_SWITCH1_PIN    = PK_1;

////////////////////////////////////////////////////////
// Lead Screw pins drive Motor 1 on header X9

const uint8_t LEADSCREW_MOTOR1_INA_PIN           = PL_0;
const uint8_t LEADSCREW_MOTOR1_INB_PIN           = PH_2;
const uint8_t LEADSCREW_MOTOR1_PWM_PIN           = PF_1;

const uint8_t LEADSCREW_TOP_LIMIT_SWITCH1_PIN       = PE_3; 
//const uint8_t LEADSCREW_RELOAD_LIMIT_SWITCHX_PIN  = xxx; // todo van horn/rausch 
//const uint8_t LEADSCREW_EMPTY_LIMIT_SWITCHX_PIN   = xxx; // todo van horn/rausch 
//const uint8_t LEADSCREW_DEPLOY_LIMIT_SWITCHX_PIN  = xxx; // todo van horn/rausch  
const uint8_t LEADSCREW_BOTTOM_LIMIT_SWITCH2_PIN    = PE_2;

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
  
  LeadScrewMotor.begin(LEADSCREW_MOTOR1_INA_PIN, LEADSCREW_MOTOR1_INB_PIN, LEADSCREW_MOTOR1_PWM_PIN);
  GenevaMotor.begin(   GENEVA_MOTOR2_INA_PIN,    GENEVA_MOTOR2_INB_PIN,    GENEVA_MOTOR2_PWM_PIN   );   
  DrillMotor.begin(    DRILL_MOTOR1_INA_PIN,     DRILL_MOTOR1_INB_PIN,     DRILL_MOTOR1_PWM_PIN    );  
 
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
    case DRILL_OPEN_LOOP:
    {
      int drill_speed = *(int16_t*)(data);       
      DrillMotor.drive(drill_speed);       
      Watchdog.clear();
      break;
    }
    
    case GENEVA_OPEN_LOOP:
    {
      int geneva_speed = *(int16_t*)(data); 
      GenevaMotor.drive(geneva_speed);       
      Watchdog.clear();
      break;
    }
    
    case LEADSCREW_OPEN_LOOP:
    {
      int leadscrew_speed = *(int16_t*)(data);
      LeadScrewMotor.drive(leadscrew_speed);       
      Watchdog.clear();
      break;
    }
    
    case GENEVA_TO_LIMIT_SWITCH:
    {
      // todo van horn/rausch/skelton
      break;
    }
    
    case LEADSCREW_TO_LIMIT_SWITCH: 
    
      /* // todo van horn/rausch/skelton

      // here is a simple stateless sketch that just sweeps up and down till found, but we should probably add a state machine
      int leadscrew_limit_switch = *(uint8_t*)(data);          
      int leadscrew_limit_switch_pin;  
     
      switch(leadscrew_limitswitch)
      {
        case LEADSCREW_BOTTOM_LIMIT_SWITCH:   
          leadscrew_limit_switch_pin = 
          
        case LEADSCREW_DROPOFF_LIMIT_SWITCH:   
          leadscrew_limit_switch_pin =  
          
        case LEADSCREW_EMPTY_SWITCH:   
          leadscrew_limit_switch_pin =  
          
        case LEADSCREW_RELOAD_LIMIT_SWITCH:   
          leadscrew_limit_switch_pin =  
          
        case LEADSCREW_TOP_LIMIT_SWITCH:   
          leadscrew_limit_switch_pin =  
          
        default:
          break;    
      }
      
      bool target_limit_reached = false;          
      bool bottom_limit_reached = false;
      bool top_limit_reached    = false;
      
      int  leadscrew_speed = 1000;
      
      while(!target_limit_reached)
      {
        bool target_limit_reached = digitalRead();
        bool bottom_limit_reached = digitalRead();
        bool top_limit_reached    = digitalRead();
      
        if((!top_bottom_reached) && (!bottom_limit_reached))
        {   
          LeadScrewMotor.drive(leadscrew_speed);   
        } else 
        {
          LeadScrewMotor.brake();          
          leadscrew_speed = -leadscrew_speed;        
          LeadScrewMotor.drive(leadscrew_speed); 
          delay(10);
        }          
      }*/
      
    default:
      break;
  }
}

//////////////////////////

void estop()
{
  DrillMotor.brake(0);  
  GenevaMotor.brake(0);
  LeadScrewMotor.brake(0);      
}