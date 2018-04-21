#include "main.h"
#include <stdio.h>

ControlState currentControlState;
uint32_t screwDestination = 0;

VNH5019 drillDriver(DRILLMOTOR1_GEN, DRILLMOTOR1_PWMPIN, DRILLMOTOR1_INAPIN, DRILLMOTOR1_INBPIN, false);
VNH5019 screwDriver(SCREWMOTOR1_GEN, SCREWMOTOR1_PWMPIN, SCREWMOTOR1_INAPIN, SCREWMOTOR1_INBPIN, false);
Ma3Encoder12b encoder(ENCODERGEN, ENCODERPIN);
PIAlgorithm alg(AlgK, AlgI, AlgDt, &encoder);

SingleMotorJoint drillInterface(InputPowerPercent, &drillDriver);
SingleMotorJoint screwInterface(InputPowerPercent, &screwDriver);
RoveTimer_Handle loopTimer;

/**
 * main.cpp
 */
int main()
{
  setup();
  while(1)
  {
    processBaseStationCommands();
  }
}

void setup()
{
  roveComm_Begin(ip[0], ip[1], ip[2], ip[3]);

  loopTimer = setupTimer(7, TimerPeriodicInterrupt, AlgDt * 1000000, closedLoopHandler);

  for(int i = 0; i < 20; i++)
  {
    encoder.getFeedback();
    delay(4);
  }
}

void processBaseStationCommands()
{
  uint16_t commandId = 0;
  size_t commandSize = 0;
  char commandData[100];

  roveComm_GetMsg(&commandId, &commandSize, commandData);
  if(commandId != 0) //returns commandId == 0 if it didn't get any message
  {
    switch(commandId)
    {
      case DrillId:
        moveDrill(*((int16_t*)commandData));
        break;

      case ScrewId:
        moveScrew(*((int16_t*)commandData));
        break;

      case ScrewPositionId:
        setScrewDestination(*((float*)commandData));
        break;

      default:
        break;
    }
  }
}

void moveDrill(int16_t moveValue)
{
  drillInterface.runOutputControl(moveValue);
}

void moveScrew(int16_t moveValue)
{
  if(currentControlState != OpenLoop)
  {
    switchToOpenLoopScrew();
  }

  if(checkLimSwitchHit(SCREW_LIM_UP_PIN))
  {
    if(moveValue > 0)
    {
      moveValue = 0;
    }
  }
  else if(checkLimSwitchHit(SCREW_LIM_DOWN_PIN))
  {
    if(moveValue < 0)
    {
      moveValue = 0;
    }
  }

  screwInterface.runOutputControl(moveValue);
}

void switchToClosedLoopScrew()
{
  screwInterface.switchModules(InputPosition, &alg);
  screwDestination = encoder.getFeedback();
  startTimer(loopTimer);
  currentControlState = ClosedLoop;
}

void switchToOpenLoopScrew()
{
  screwInterface.removeIOConverter(InputPowerPercent);
  stopTimer(loopTimer);
  currentControlState = OpenLoop;
}

void setScrewDestination(float degreesDestination)
{
  if(currentControlState != ClosedLoop)
  {
    switchToClosedLoopScrew();
  }

  screwDestination = degreesDestination * DEGREES_TO_POS;
}

bool checkLimSwitchHit(uint8_t pin)
{
  return (digitalPinRead(pin) == false);
}

void closedLoopHandler()
{
  screwInterface.runOutputControl(screwDestination);
}
