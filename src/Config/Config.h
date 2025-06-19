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
#define MM_PER_INCH             25.4     // Millimeters per inch
#define STEPS_PER_INCH          (STEPS_PER_MM * MM_PER_INCH)    // Steps per inch

// Motion settings
#define HOMING_SPEED            500      // Steps/second for homing
#define HOMING_OFFSET_INCHES    0.2      // Offset after homing trigger
#define HOMING_OFFSET_MM        (HOMING_OFFSET_INCHES * MM_PER_INCH)   // Convert to mm
#define HOMING_OFFSET_STEPS     (HOMING_OFFSET_MM * STEPS_PER_MM) // Convert to steps

//* ************************************************************************
//* ************************ CUTTING CYCLE SETTINGS **********************
//* ************************************************************************

// Cutting cycle movements
#define APPROACH_DISTANCE_INCHES    5.0     // Approach distance
#define CUTTING_DISTANCE_INCHES     3.0     // Cutting distance  
#define DROPOFF_DISTANCE_INCHES     10.0    // Drop off distance
#define CLAMP_RETRACT_DISTANCE_INCHES 7.0   // Distance when clamps retract during drop off

// Convert distances to steps
#define APPROACH_DISTANCE_STEPS     (APPROACH_DISTANCE_INCHES * STEPS_PER_INCH)
#define CUTTING_DISTANCE_STEPS      (CUTTING_DISTANCE_INCHES * STEPS_PER_INCH)
#define DROPOFF_DISTANCE_STEPS      (DROPOFF_DISTANCE_INCHES * STEPS_PER_INCH)
#define CLAMP_RETRACT_DISTANCE_STEPS (CLAMP_RETRACT_DISTANCE_INCHES * STEPS_PER_INCH)

// Cutting cycle speeds (steps/second)
#define APPROACH_SPEED              5000    // Approach speed
#define CUTTING_SPEED               1000    // Cutting speed
#define DROPOFF_SPEED               30000   // Drop off speed
#define RETURN_SPEED                30000   // Return to home speed

// Timing
#define CLAMP_ENGAGE_DELAY_MS       500     // Wait time after clamp engagement

//* ************************************************************************
//* ************************ STATE DEFINITIONS ****************************
//* ************************************************************************

enum MachineState {
    STATE_HOMING,
    STATE_IDLE,
    STATE_CUTTING_CYCLE
};

// Cutting cycle substates
enum CuttingSubstate {
    SUBSTATE_APPROACH,
    SUBSTATE_CUTTING,
    SUBSTATE_DROPOFF,
    SUBSTATE_RETURN
};

// Global state variable declaration
extern MachineState currentState;
extern CuttingSubstate currentSubstate;

#endif 