#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <FastAccelStepper.h>
#include <Bounce2.h>
#include "Config/Config.h"

//* ************************************************************************
//* ************************ GLOBAL OBJECTS *******************************
//* ************************************************************************
//! Global hardware objects used throughout the application

extern FastAccelStepperEngine stepperEngine;
extern FastAccelStepper *stepper;
extern Bounce2::Button startButton;
extern Bounce2::Button homingSwitch;

//* ************************************************************************
//* ************************ STATE FUNCTION DECLARATIONS *****************
//* ************************************************************************
//! State machine function declarations

// State functions
void homingState();
void idleState();
void cuttingCycleState();

// Utility functions
bool initializeHardware();
void extendClamp();
void retractClamp();

#endif 