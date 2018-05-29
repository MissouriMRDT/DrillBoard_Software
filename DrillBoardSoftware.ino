/*DrillBoard Software
 * Rev 1 2018
 * Used with 4 Drilboard Rev 1 2018 (2 per booster)
 * Andrew Van Horn, Judah Schad * 
 */
 #include "RoveWare.h"

////////////////////////////////////////
// Drill pins drive Motor 1 on header X7
const uint8_t DRILL_INA_PIN  = PH_0;
const uint8_t DRILL_INB_PIN  = PP_5;
const uint8_t DRILL_PWM_PIN  = PG_1;

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

const uint8_t LEADSCREW_LIMIT_SWITCH_1_BOTTOM_PIN  = PE_3; //Uses LS 1 on X9
const uint8_t LEADSCREW_LIMIT_SWITCH_2_DEPLOY_PIN  = PE_2; //Uses LS 2 on X9
const uint8_t LEADSCREW_LIMIT_SWITCH_3_RELOAD_PIN  = PE_1; //Uses LS 3 on X9
const uint8_t LEADSCREW_LIMIT_SWITCH_4_TOP_PIN   = PE_0; //Uses LS 4 on X9

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//State Variables
enum GENEVA_POSITIONS    { GENEVA_OPENLOOP=0,    GENEVA_SLOT_1_HOME=1, GENEVA_SLOT_2=2,   GENEVA_SLOT_3=3,    GENEVA_SLOT_4=4,    GENEVA_SLOT_5=5, GENEVA_SLOT_6=6 }; 
enum LEADSCREW_POSITIONS { LEADSCREW_OPENLOOP=0, LEADSCREW_BOTTOM=1,   LEADSCREW_DEPLOY=2, LEADSCREW_RELOAD=3, LEADSCREW_EMPTY=4, LEADSCREW_TOP=4 }; 

uint8_t geneva_command_position    = GENEVA_OPENLOOP;
uint8_t geneva_present_position    = GENEVA_SLOT_1_HOME; 
bool geneva_between_positions = 1;

uint8_t leadscrew_command_position;
uint8_t leadscrew_present_position;

const int LEADSCREW_TO_POSITION_SPEED = 500;
const int GENEVA_TO_POSITION_SPEED    = 500;  //I'm getting strange results for drive(-1000) it stops


///////////////////////////
//Motor Speed Variables
int16_t drill_speed;
int16_t geneva_speed;
int16_t leadscrew_speed;
/////////////////////////////////////

RoveCommEthernetUdp  RoveComm;

RoveWatchdog         Watchdog;

RoveVnh5019          DrillMotor;
RoveVnh5019          GenevaMotor;
RoveVnh5019          LeadScrewMotor;

//////////////////////////////////////////////////////////////////////////////////


void leadscrewToPosition(uint8_t leadscrew_present_position, uint8_t leadscrew_command_position);
void genevaToPosition(   uint8_t geneva_present_position,    uint8_t geneva_command_position);

uint8_t leadscrewGetPosition(uint8_t leadscrew_present_position);
uint8_t genevaGetPosition(   uint8_t geneva_present_position, int geneva_speed);

void leadscrewToPosition(uint8_t leadscrew_present_position, uint8_t leadscrew_command_position);
void genevaToPosition(   uint8_t geneva_present_position,    uint8_t geneva_command_position   );

bool drivingPastBottomLimit(uint8_t bottom_limit_switch_pin, int16_t command_speed);
bool drivingPastTopLimit(   uint8_t top_limit_switch_pin,    int16_t command_speed);

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
  
  pinMode(GENEVA_INA_PIN,    OUTPUT);
  pinMode(GENEVA_INB_PIN,    OUTPUT);
  pinMode(GENEVA_PWM_PIN,    OUTPUT);
  
  pinMode(LEADSCREW_INA_PIN, OUTPUT);
  pinMode(LEADSCREW_INB_PIN, OUTPUT);
  pinMode(LEADSCREW_PWM_PIN, OUTPUT);

  pinMode(GENEVA_LIMIT_SWITCH_PIN,      INPUT);
  pinMode(CAROUSEL_LIMIT_SWITCH_PIN,    INPUT);

  pinMode(LEADSCREW_LIMIT_SWITCH_1_BOTTOM_PIN, INPUT);
  pinMode(LEADSCREW_LIMIT_SWITCH_2_DEPLOY_PIN, INPUT);
  pinMode(LEADSCREW_LIMIT_SWITCH_3_RELOAD_PIN, INPUT);
  pinMode(LEADSCREW_LIMIT_SWITCH_4_TOP_PIN,    INPUT);
   
  LeadScrewMotor.begin(LEADSCREW_INA_PIN, LEADSCREW_INB_PIN, LEADSCREW_PWM_PIN);
  GenevaMotor.begin(   GENEVA_INA_PIN,    GENEVA_INB_PIN,    GENEVA_PWM_PIN   );   
  DrillMotor.begin(    DRILL_INA_PIN,     DRILL_INB_PIN,     DRILL_PWM_PIN    );  
 
  Watchdog.begin(estop, 150);
}

//////////////////////////////////////////////////////////////////////////////////////////

void loop()
{   
  delay(10);
  uint16_t data_id   = 0; 
  size_t   data_size = 0; 
  uint8_t  data[2];

  RoveComm.read(&data_id, &data_size, data);

  data_id = LEADSCREW_OPEN_LOOP;//Debug
  

  switch(data_id)
  {      
    case DRILL_OPEN_LOOP:
      drill_speed = *(int16_t*)(data);   
      DrillMotor.drive(geneva_speed);
      Watchdog.clear();
      break; 
	  
    case LEADSCREW_OPEN_LOOP:
      leadscrew_speed = (int16_t)(data[1] << 8 || data[0]);		
        leadscrew_speed = 200; //Debug
      if(  (!drivingPastBottomLimit(LEADSCREW_LIMIT_SWITCH_1_BOTTOM_PIN, leadscrew_speed))
	    && (!drivingPastTopLimit(   LEADSCREW_LIMIT_SWITCH_4_TOP_PIN,    leadscrew_speed)))
  	  {
        LeadScrewMotor.drive(leadscrew_speed); 
      }
      else
      {
        LeadScrewMotor.brake(0);
      }	  
	  leadscrew_command_position = LEADSCREW_OPENLOOP; 
      Watchdog.clear();	  
      break;

    case GENEVA_OPEN_LOOP:
      geneva_speed = *(int16_t*)(data); 
      GenevaMotor.drive(geneva_speed);
      geneva_command_position = GENEVA_OPENLOOP;      
      Watchdog.clear();
      break;
    
	
    case LEADSCREW_TO_POSITION:
       leadscrew_command_position = *(int8_t*)data;
       Watchdog.clear();	  	      
       break;
    
    case GENEVA_TO_POSITION:
      geneva_command_position = *(int8_t*)data;      
      Watchdog.clear();
      break; 
	  
    default:
      break;
  }

  leadscrew_present_position = leadscrewGetPosition(leadscrew_present_position);
  geneva_present_position    = genevaGetPosition(geneva_present_position, geneva_speed);

  Serial.print("Present Pos:");
  Serial.println(leadscrew_present_position);
  
  RoveComm.write(LEADSCREW_AT_POSITION, sizeof(leadscrew_present_position), &leadscrew_present_position);
  RoveComm.write(GENEVA_AT_POSITION,    sizeof(geneva_present_position),    &geneva_present_position);

  uint8_t temp = leadscrewGetPosition(0);
  RoveComm.write(LEADSCREW_LIMIT_SWITCH_TRIGGERED,   sizeof(uint8_t), &temp);
  temp = digitalRead(GENEVA_LIMIT_SWITCH_PIN);
  RoveComm.write(GENEVA_LIMIT_SWITCH_TRIGGERED,      sizeof(uint8_t), &temp);
  temp = digitalRead(CAROUSEL_LIMIT_SWITCH_PIN);
  RoveComm.write(CAROUSEL_LIMIT_SWITCH_TRIGGERED,    sizeof(uint8_t), &temp);

  delay(10);
  if(geneva_command_position != GENEVA_OPENLOOP)
  {
    genevaToPosition(geneva_present_position, geneva_command_position);
  }
  if(leadscrew_command_position != LEADSCREW_OPENLOOP)
  {
    leadscrewToPosition(leadscrew_present_position, leadscrew_command_position);
  }  
}

///////////////////////////////////////////////////////////////////////////////////

bool drivingPastBottomLimit(uint8_t bottom_limit_switch_pin, int16_t command_speed)
{
  return (digitalRead(bottom_limit_switch_pin) && (command_speed < 0));
}

//////////////////////////////////////////////////////////////////////////////

bool drivingPastTopLimit(uint8_t top_limit_switch_pin, int16_t command_speed)
{
  return (digitalRead(top_limit_switch_pin) && (command_speed > 0));
}

/////////////////////////////////////////////////////////////////////////////

uint8_t leadscrewGetPosition(uint8_t leadscrew_present_position)
{    
  if(digitalRead(LEADSCREW_LIMIT_SWITCH_4_TOP_PIN))
  {
    leadscrew_present_position = LEADSCREW_TOP;
	
  } 
  else if(digitalRead(LEADSCREW_LIMIT_SWITCH_1_BOTTOM_PIN)) 
  {
    leadscrew_present_position = LEADSCREW_BOTTOM;
  }
  else if(leadscrew_speed < 0)
  {
    if(digitalRead(LEADSCREW_LIMIT_SWITCH_2_DEPLOY_PIN))
    {
      leadscrew_present_position = LEADSCREW_DEPLOY;
    }
    else if(digitalRead(LEADSCREW_LIMIT_SWITCH_3_RELOAD_PIN))
    {
      leadscrew_present_position = LEADSCREW_RELOAD;
    }
  }
  else if(leadscrew_speed > 0)
  {
    if(digitalRead(LEADSCREW_LIMIT_SWITCH_2_DEPLOY_PIN))
    {
      while(digitalRead(LEADSCREW_LIMIT_SWITCH_2_DEPLOY_PIN))
      {
        LeadScrewMotor.drive(leadscrew_speed);
      }
      leadscrew_present_position = LEADSCREW_DEPLOY;
    }

    if(digitalRead(LEADSCREW_LIMIT_SWITCH_3_RELOAD_PIN))
    {
      while(digitalRead(LEADSCREW_LIMIT_SWITCH_3_RELOAD_PIN))
      {
        LeadScrewMotor.drive(leadscrew_speed);
      }
      leadscrew_present_position = LEADSCREW_RELOAD;
    }
    
  }

  
  return leadscrew_present_position;
}

////////////////////////////////////////////////////////////////////////////////////

uint8_t genevaGetPosition(uint8_t geneva_present_position, int geneva_speed)
{
  if(digitalRead(GENEVA_LIMIT_SWITCH_PIN) && digitalRead(CAROUSEL_LIMIT_SWITCH_PIN))
  {
    geneva_present_position = GENEVA_SLOT_1_HOME;
  
  //Increment up if the motor is going on a limit switch moving forward, only increment once on the first transition from between state
  } else if(digitalRead(GENEVA_LIMIT_SWITCH_PIN) && geneva_between_positions && (geneva_speed > 0))
  {
    geneva_present_position++;
	
  //Increment down if the mtor is going off a limit switch moving backward, only increment once on the first transition from between state
  } else if(!digitalRead(GENEVA_LIMIT_SWITCH_PIN) && !geneva_between_positions && (geneva_speed < 0))
  {
    geneva_present_position--;
  }
   geneva_between_positions = !digitalRead(GENEVA_LIMIT_SWITCH_PIN);

  //Rollover Position////////////////////////////
  if(geneva_present_position > GENEVA_SLOT_6)
	{		
    geneva_present_position = GENEVA_SLOT_1_HOME;
  }
  if(geneva_present_position < GENEVA_SLOT_1_HOME)
  {   
    geneva_present_position = GENEVA_SLOT_6;
  }
  return(geneva_present_position);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void leadscrewToPosition(uint8_t leadscrew_present_position, uint8_t leadscrew_command_position)
{
  if(leadscrew_present_position == leadscrew_command_position)     
  {
	LeadScrewMotor.brake(0);  
  } else if ((leadscrew_present_position < leadscrew_command_position) 
	     && (!drivingPastTopLimit(LEADSCREW_LIMIT_SWITCH_4_TOP_PIN,        LEADSCREW_TO_POSITION_SPEED)))
  {
    LeadScrewMotor.drive(LEADSCREW_TO_POSITION_SPEED);  
  } else if ((leadscrew_present_position > leadscrew_command_position) 
	     && (!drivingPastBottomLimit(LEADSCREW_LIMIT_SWITCH_1_BOTTOM_PIN, -LEADSCREW_TO_POSITION_SPEED)))
  {
    LeadScrewMotor.drive(-LEADSCREW_TO_POSITION_SPEED);
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void genevaToPosition(uint8_t geneva_present_position, uint8_t geneva_command_position)
{
  int direction_no_overflow = 0;
  if (geneva_present_position < geneva_command_position)
  {
    direction_no_overflow = 1;
  } else
  {
    direction_no_overflow = -1;
  }

  if((geneva_present_position == geneva_command_position) && digitalRead(GENEVA_LIMIT_SWITCH_PIN))     
  {
    GenevaMotor.brake(0);  
    geneva_speed = 0;
  } else if(abs(geneva_present_position-geneva_command_position)<=2)
  {
    geneva_speed = direction_no_overflow*GENEVA_TO_POSITION_SPEED;
    GenevaMotor.drive(geneva_speed);
  }else
  {
    geneva_speed = -1*direction_no_overflow*GENEVA_TO_POSITION_SPEED;
    GenevaMotor.drive(geneva_speed);;
  }
}

//////////////////////////
void estop()
{
  DrillMotor.brake(0);  
  GenevaMotor.brake(0);
  LeadScrewMotor.brake(0);      
}

