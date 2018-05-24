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
const uint8_t LEADSCREW_LIMIT_SWITCH_4_EMPTY_PIN   = PE_0; //Uses LS 4 on X9
const uint8_t LEADSCREW_LIMIT_SWITCH_5_TOP_PIN     = PB_5; //Uses LS 3 on X7

////////////////////////////////////////////////////////////////////////////////
//command_data for the GENEVA_AT_POSITION data_id => todo move to RoveManifest.h

const uint8_t GENEVA_BETWEEN_POSITIONS = 0;   
const uint8_t GENEVA_SLOT_1_HOME       = 1; 
const uint8_t GENEVA_SLOT_2            = 2;   
const uint8_t GENEVA_SLOT_3            = 3;    
const uint8_t GENEVA_SLOT_4            = 4;    
const uint8_t GENEVA_SLOT_5            = 5; 
const uint8_t GENEVA_SLOT_6            = 6;
 
///////////////////////////////////////////////////////////////////////////////////
//command_data for the LEADSCREW_AT_POSITION data_id => todo move to RoveManifest.h

const uint8_t LEADSCREW_BETWEEN_POSITIONS = 0; 
const uint8_t LEADSCREW_BOTTOM            = 1, 
const uint8_t LEADSCREW_DEPLOY            = 3, 
const uint8_t LEADSCREW_EMPTY             = 5, 
const uint8_t LEADSCREW_RELOAD            = 7, 
const uint8_t LEADSCREW_TOP               = 9 
 
const int LEADSCREW_TO_POSITION_SPEED = 500;
const int GENEVA_TO_POSITION_SPEED    = 500;

///////////////////

enum CONTROL_MODES    
{ 
  OPEN_LOOP   = 0,    
  TO_POSITION = 1
}; 

////////////////////////////////////////////

uint8_t geneva_control_mode     = OPEN_LOOP;
uint8_t leadscrew_control_mode  = OPEN_LOOP;

uint8_t geneva_command_position;
uint8_t geneva_present_position; 
uint8_t geneva_last_slot_position; 

uint8_t leadscrew_command_position;
uint8_t leadscrew_present_position;

uint16_t drill_speed;
uint16_t geneva_speed;
uint16_t leadscrew_speed;

/////////////////////////////////////

RoveCommEthernetUdp  RoveComm;

RoveWatchdog         Watchdog;

RoveVnh5019          DrillMotor;
RoveVnh5019          GenevaMotor;
RoveVnh5019          LeadScrewMotor;

//////////////////////////////////////////////////////////////////////////////////

uint8_t leadscrewGetPosition();
uint8_t genevaGetPosition(uint8_t geneva_last_position, uint16_t geneva_speed);

void leadscrewToPosition(uint8_t leadscrew_last_position, uint8_t leadscrew_command_position);
void genevaToPosition(   uint8_t geneva_last_position,    uint8_t geneva_command_position   );

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
  pinMode(LEADSCREW_LIMIT_SWITCH_4_EMPTY_PIN,  INPUT);
  pinMode(LEADSCREW_LIMIT_SWITCH_5_TOP_PIN,    INPUT);
  
  LeadScrewMotor.begin(LEADSCREW_INA_PIN, LEADSCREW_INB_PIN, LEADSCREW_PWM_PIN);
  GenevaMotor.begin(   GENEVA_INA_PIN,    GENEVA_INB_PIN,    GENEVA_PWM_PIN   );   
  DrillMotor.begin(    DRILL_INA_PIN,     DRILL_INB_PIN,     DRILL_PWM_PIN    );  
 
  Watchdog.begin(estop, 150);
}

//////////////////////////////////////////////////////////////////////////////////////////

void loop()
{   
  uint16_t data_id   = 0; 
  size_t   data_size = 0; 
  uint8_t  data[2];

  RoveComm.read(&data_id, &data_size, data);

  switch(data_id)
  {      
    case DRILL_OPEN_LOOP:
      drill_speed = *(int16_t*)(data);   
      DrillMotor.drive(geneva_speed);
      Watchdog.clear();
      break;
	  
    case LEADSCREW_OPEN_LOOP:
      leadscrew_speed = *(int16_t*)(data);		  
      if(  (!drivingPastBottomLimit(LEADSCREW_LIMIT_SWITCH_1_BOTTOM_PIN, leadscrew_speed))
	    && (!drivingPastTopLimit(   LEADSCREW_LIMIT_SWITCH_5_TOP_PIN,    leadscrew_speed)))
	  {
        LeadScrewMotor.drive(leadscrew_speed); 
      }  
	  leadscrew_control_mode = OPENLOOP;
      Watchdog.clear();	  
      break;

    case GENEVA_OPEN_LOOP:
      geneva_speed = *(int16_t*)(data); 
      GenevaMotor.drive(geneva_speed);
      geneva_control_mode = OPENLOOP;      
      Watchdog.clear();
      break;
    
    case LEADSCREW_TO_POSITION:
       leadscrew_command_position = *(int8_t*)data;
	   leadscrew_control_mode = TO_POSITION;
       Watchdog.clear();	   
	   // If Andrew wants to nudge off the limit switch automatically, make this a very small delay
	   /*if(leadscrew_command_position > leadscrew_present_position)
	   {
		 LeadScrewMotor.drive(LEADSCREW_TO_POSITION_SPEED); 
		 delay(LEADSCREW_MOVE_OFF_LIMITSWITCH_DELAY);
		 
	   } else if( leadscrew_command_position < leadscrew_present_position)
	   {
		 LeadScrewMotor.drive(-LEADSCREW_TO_POSITION_SPEED); 
		 delay(LEADSCREW_MOVE_OFF_LIMITSWITCH_DELAY);
	   }*/	   
       break;
    
    case GENEVA_TO_POSITION:
      geneva_command_position = *(int8_t*)data;   
      geneva_control_mode = TO_POSITION;	  
      Watchdog.clear();	
      // If Andrew wants to nudge off the limit switch automatically, make this a very small delay	  
	  /*if(geneva_command_position > geneva_last_slot_position)
	  {
	    GenevaMotor.drive(GENEVA_TO_POSITION_SPEED); 
	    delay(GENEVA_MOVE_OFF_LIMITSWITCH_DELAY);

	  } else if( geneva_command_position > geneva_last_slot_position)
	  {
		GenevaMotor.drive(-GENEVA_TO_POSITION_SPEED); 
		delay(GENEVA_MOVE_OFF_LIMITSWITCH_DELAY);
	  }*/	   
      break; 
	  
    default:
      break;
  }
  
  leadscrew_present_position = leadscrewGetPosition();
  geneva_present_position    = genevaGetPosition(geneva_present_position, geneva_speed);
  
  RoveComm.write(LEADSCREW_AT_POSITION, sizeof(leadscrew_present_position), &leadscrew_present_position);
  RoveComm.write(GENEVA_AT_POSITION,    sizeof(geneva_present_position),    &geneva_present_position);
  
  if(geneva_control_mode == TO_POSITION)
  {
	if(geneva_present_position != BETWEEN_POSITIONS)
    {
      geneva_last_slot_position = geneva_present_position;
    }
    genevaToPosition(geneva_last_slot_position, geneva_command_position);
  }
  
  if(leadscrew_control_mode == TO_POSITION)
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

uint8_t leadscrewGetPosition()
{    
  if(digitalRead(LEADSCREW_LIMIT_SWITCH_5_TOP_PIN))
  {
    leadscrew_present_position = LEADSCREW_TOP;
	
  } else if(digitalRead(LEADSCREW_LIMIT_SWITCH_4_EMPTY_PIN))
  {
    leadscrew_present_position = LEADSCREW_EMPTY;
	  
  } else if(digitalRead(LEADSCREW_LIMIT_SWITCH_3_RELOAD_PIN))
  {
    leadscrew_present_position = LEADSCREW_RELOAD;
	  
  } else if(digitalRead(LEADSCREW_LIMIT_SWITCH_2_DEPLOY_PIN)) 
  {
    leadscrew_present_position = LEADSCREW_DEPLOY;
	  
  } else if(digitalRead(LEADSCREW_LIMIT_SWITCH_1_BOTTOM_PIN)) 
  {
    leadscrew_present_position = LEADSCREW_BOTTOM;
  } else 
  {
	leadscrew_present_position = LEADSCREW_BETWEEN_POSITIONS;
  }
  
  return leadscrew_present_position;
}

////////////////////////////////////////////////////////////////////////////////////

uint8_t genevaGetPosition(uint8_t geneva_last_position, uint16_t geneva_speed
{
  if(digitalRead(GENEVA_LIMIT_SWITCH_PIN) && digitalRead(CAROUSEL_LIMIT_SWITCH_PIN))
  {
    geneva_last_position = GENEVA_SLOT_1_HOME;
  
  //Increment up if the motor is going on a limit switch moving forward, only increment once on the first transition from between state
  } else if(digitalRead(GENEVA_LIMIT_SWITCH_PIN) && (geneva_last_position == GENEVA_BETWEEN_POSITIONS) && (geneva_speed > 0))
  {
    geneva_last_position++;
	
    //Rollover Position////////////////////////////
    if(geneva_last_position > GENEVA_SLOT_6)
    {		
      geneva_last_position = GENEVA_SLOT_1_HOME;
	}
	
  //Increment down if the mtor is going off a limit switch moving backward, only increment once on the first transition from between state
  } else if(!digitalRead(GENEVA_LIMIT_SWITCH_PIN) && geneva_last_position == GENEVA_BETWEEN_POSITIONS) && (geneva_speed < 0))
  {
    geneva_last_position--;
	
	//Rollover Position////////////////////////////
    if(geneva_last_position == GENEVA_SLOT_1_HOME) // Andrew notice the protection needed here
    {		
      geneva_last_position = GENEVA_SLOT_6;
	}
  } else 
  {
	geneva_last_position = GENEVA_BETWEEN_POSITIONS;  
  }

  return geneva_last_position;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void leadscrewToPosition(uint8_t leadscrew_last_position, uint8_t leadscrew_command_position)
{
  if(leadscrew_last_position == leadscrew_command_position)     
  {
	LeadScrewMotor.brake(0);  
	
  } else if ((leadscrew_last_position < leadscrew_command_position) 
	     && (!drivingPastBottomLimit(LEADSCREW_LIMIT_SWITCH_1_BOTTOM_PIN, LEADSCREW_TO_POSITION_SPEED))) // Andrew notice the protection needed here
  {
    LeadScrewMotor.drive(LEADSCREW_TO_POSITION_SPEED);  
	
  } else if ((leadscrew_last_position > leadscrew_command_position) 
	     && (!drivingPastBottomLimit(LEADSCREW_LIMIT_SWITCH_1_BOTTOM_PIN, -LEADSCREW_TO_POSITION_SPEED))) // Andrew notice the protection needed here
  {
    LeadScrewMotor.drive(-LEADSCREW_TO_POSITION_SPEED);
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void genevaToPosition(uint8_t geneva_last_position, uint8_t geneva_command_position)
{
  if(geneva_last_position == geneva_command_position)     
  {
    GenevaMotor.brake(0); 
	
  } else if (geneva_last_position < geneva_command_position)
  {
    GenevaMotor.drive(GENEVA_TO_POSITION_SPEED); 
	
  } else if (geneva_last_position > geneva_command_position)
  {
    GenevaMotor.drive(-GENEVA_TO_POSITION_SPEED);
  }
}

//////////////////////////
void estop()
{
  DrillMotor.brake(0);  
  GenevaMotor.brake(0);
  LeadScrewMotor.brake(0);      
}

