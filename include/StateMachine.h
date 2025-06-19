#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <FastAccelStepper.h>
#include <Bounce2.h>
#include "../src/Config/Config.h"

//* ************************************************************************
//* ************************ GLOBAL OBJECTS *******************************
//* ************************************************************************

extern FastAccelStepperEngine stepperEngine;
extern FastAccelStepper *stepper;
extern Bounce2::Button startButton;
extern Bounce2::Button homingSwitch;

//* ************************************************************************
//* ************************ STATE FUNCTION DECLARATIONS *****************
//* ************************************************************************

// State functions
void homingState();
void idleState();
void cuttingCycleState();

// Utility functions
void initializeHardware();
void extendClamp();
void retractClamp();

#endif 