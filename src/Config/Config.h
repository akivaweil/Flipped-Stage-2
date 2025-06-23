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

// Motor specifications
#define STEPS_PER_REV           400      // Steps per revolution
#define PULLEY_TEETH            60       // 60 tooth pulley
#define BELT_PITCH_MM           2.0      // 2GT belt (2mm pitch)

// Calculated values
#define MM_PER_REV              (PULLEY_TEETH * BELT_PITCH_MM)  // 120mm per revolution
#define STEPS_PER_MM            (STEPS_PER_REV / MM_PER_REV)    // Steps per mm
#define MM_PER_INCH             25.4     // Millimeters per inch
#define STEPS_PER_INCH          (STEPS_PER_MM * MM_PER_INCH)    // Steps per inch

// Motion settings
#define HOMING_SPEED            300      // Steps/second for homing
#define HOMING_OFFSET_INCHES    .3       // Offset after homing trigger
#define HOMING_OFFSET_MM        (HOMING_OFFSET_INCHES * MM_PER_INCH)   // Convert to mm
#define HOMING_OFFSET_STEPS     (HOMING_OFFSET_MM * STEPS_PER_MM)      // Convert to steps

// Acceleration settings
#define STEPPER_ACCELERATION    40000    // Steps/second² for all movements
#define APPROACH_ACCELERATION   10000     // Steps/second² for approach movement - slower for precision

//* ************************************************************************
//* ************************ CUTTING CYCLE SETTINGS **********************
//* ************************************************************************

// Cutting cycle absolute positions (from home offset at 0)
#define APPROACH_POSITION_INCHES    3.5     // Approach position
#define CUTTING_POSITION_INCHES     9.5     // Cutting position  
#define EARLY_CLAMP_RELEASE_POSITION_INCHES 18.0    // Early clamp release position
#define DROPOFF_POSITION_INCHES     24.5    // Drop off position
#define HOME_POSITION_INCHES        0.0     // Home offset position
#define EMERGENCY_RECOVERY_POSITION_INCHES  -1.0    // Emergency recovery position (1 inch away from home)

// Convert absolute positions to steps
#define APPROACH_POSITION_STEPS     (APPROACH_POSITION_INCHES * STEPS_PER_INCH)    // 3.5" from start
#define CUTTING_POSITION_STEPS      (CUTTING_POSITION_INCHES * STEPS_PER_INCH)     // 9.5" from start  
#define EARLY_CLAMP_RELEASE_POSITION_STEPS (EARLY_CLAMP_RELEASE_POSITION_INCHES * STEPS_PER_INCH) // 12.0" from start
#define DROPOFF_POSITION_STEPS      (DROPOFF_POSITION_INCHES * STEPS_PER_INCH)     // 24.5" from start
#define HOME_POSITION_STEPS         (HOME_POSITION_INCHES * STEPS_PER_INCH)        // Return to home offset
#define EMERGENCY_RECOVERY_POSITION_STEPS (EMERGENCY_RECOVERY_POSITION_INCHES * STEPS_PER_INCH) // 1" away from home

// Cutting cycle speeds (steps/second)
#define APPROACH_SPEED              20000    // Approach speed
#define CUTTING_SPEED               250      // Cutting speed
#define DROPOFF_SPEED               80000    // Drop off speed
#define RETURN_SPEED                100000   // Return to home speed

// Timing
#define EMERGENCY_STOP_DELAY_MS     250      // Safety delay before emergency stop can be activated

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
    SUBSTATE_RETURN,
    SUBSTATE_EMERGENCY_RECOVERY
};

// Global state variable declaration
extern MachineState currentState;
extern CuttingSubstate currentSubstate;

#endif 