#ifndef CONFIG_H
#define CONFIG_H

#include "Pins_Definitions.h"

//* ************************************************************************
//* ************************ MACHINE CONFIGURATION ***********************
//* ************************************************************************
//! Configuration settings for Flipped Stage 2 machine

//* ************************************************************************
//* ************************ SYSTEM SETTINGS ******************************
//* ************************************************************************

#define MAIN_LOOP_DELAY_MS          10    // Main loop delay to prevent serial overflow

//* ************************************************************************
//* ************************ DEBOUNCE SETTINGS ****************************
//* ************************************************************************

#define HOMING_SWITCH_DEBOUNCE_MS   5     // Homing switch debounce interval
#define START_BUTTON_DEBOUNCE_MS    50    // Start button debounce interval

//* ************************************************************************
//* ************************ STEPPER MOTOR SETTINGS **********************
//* ************************************************************************

// Closed-loop stepper settings
#define POSITION_TOLERANCE      10       // Maximum allowed position error (steps)
#define AUTO_POSITION_CORRECTION true    // Enable automatic position correction

// Motor specifications
#define STEPS_PER_REV           400      // 200 steps per revolution
#define PULLEY_TEETH            60       // 60 tooth pulley
#define BELT_PITCH_MM           2.0      // 2GT belt (2mm pitch)

// Calculated values
#define MM_PER_REV              (PULLEY_TEETH * BELT_PITCH_MM)  // 120mm per revolution
#define STEPS_PER_MM            (STEPS_PER_REV / MM_PER_REV)    // Steps per mm
#define MM_PER_INCH             25.4     // Millimeters per inch
#define STEPS_PER_INCH          (STEPS_PER_MM * MM_PER_INCH)    // Steps per inch

// Motion settings
#define HOMING_SPEED            300      // Steps/second for homing
#define HOMING_OFFSET_INCHES    .5      // Offset after homing trigger
#define HOMING_OFFSET_MM        (HOMING_OFFSET_INCHES * MM_PER_INCH)   // Convert to mm
#define HOMING_OFFSET_STEPS     (HOMING_OFFSET_MM * STEPS_PER_MM) // Convert to steps

// Acceleration setting
#define STEPPER_ACCELERATION    60000     // Steps/second² for all movements - reduced for better step reliability
#define APPROACH_ACCELERATION   2000      // Steps/second² for approach movement - slower for precision

//* ************************************************************************
//* ************************ CUTTING CYCLE SETTINGS **********************
//* ************************************************************************

// Cutting cycle movements
#define APPROACH_DISTANCE_INCHES    3.5     // Approach distance
#define CUTTING_DISTANCE_INCHES     5.0     // Cutting distance  
#define DROPOFF_DISTANCE_INCHES     15    // Drop off distance

// Convert distances to steps
#define APPROACH_DISTANCE_STEPS     (APPROACH_DISTANCE_INCHES * STEPS_PER_INCH)
#define CUTTING_DISTANCE_STEPS      (CUTTING_DISTANCE_INCHES * STEPS_PER_INCH)
#define DROPOFF_DISTANCE_STEPS      (DROPOFF_DISTANCE_INCHES * STEPS_PER_INCH)

// Cutting cycle speeds (steps/second)
#define APPROACH_SPEED              20000    // Approach speed - reduced for better reliability
#define CUTTING_SPEED               400     // Cutting speed
#define DROPOFF_SPEED               80000    // Drop off speed - reduced for better reliability
#define RETURN_SPEED                100000    // Return to home speed - reduced for better reliability

// Timing
#define CLAMP_ENGAGE_DELAY_MS       500     // Wait time after clamp engagement
#define EMERGENCY_STOP_DELAY_MS     500     // Safety delay before emergency stop can be activated

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