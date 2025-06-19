#ifndef CONFIG_H
#define CONFIG_H

#include "Pins_Definitions.h"

//* ************************************************************************
//* ************************ MACHINE CONFIGURATION ***********************
//* ************************************************************************
//! Configuration settings for Flipped Stage 2 machine

//* ************************************************************************
//* ************************ DEBOUNCE SETTINGS ****************************
//* ************************************************************************

#define HOMING_SWITCH_DEBOUNCE_MS   5     // Homing switch debounce interval
#define START_BUTTON_DEBOUNCE_MS    20    // Start button debounce interval

//* ************************************************************************
//* ************************ STEPPER MOTOR SETTINGS **********************
//* ************************************************************************

// Motor specifications
#define STEPS_PER_REV           200      // 200 steps per revolution
#define PULLEY_TEETH            60       // 60 tooth pulley
#define BELT_PITCH_MM           2.0      // 2GT belt (2mm pitch)

// Calculated values
#define MM_PER_REV              (PULLEY_TEETH * BELT_PITCH_MM)  // 120mm per revolution
#define STEPS_PER_MM            (STEPS_PER_REV / MM_PER_REV)    // Steps per mm

// Motion settings
#define HOMING_SPEED            500      // Steps/second for homing
#define HOMING_OFFSET_INCHES    0.2      // Offset after homing trigger
#define HOMING_OFFSET_MM        (HOMING_OFFSET_INCHES * 25.4)   // Convert to mm
#define HOMING_OFFSET_STEPS     (HOMING_OFFSET_MM * STEPS_PER_MM) // Convert to steps

//* ************************************************************************
//* ************************ STATE DEFINITIONS ****************************
//* ************************************************************************

enum MachineState {
    STATE_HOMING,
    STATE_IDLE,
    STATE_CUTTING_CYCLE
};

// Global state variable declaration
extern MachineState currentState;

#endif 