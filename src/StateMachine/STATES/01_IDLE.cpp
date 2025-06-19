#include "StateMachine.h"

//* ************************************************************************
//* ************************ IDLE ***************************
//* ************************************************************************
//! Idle state: Wait for start button press to begin cutting cycle

void idleState() {
    static bool firstEntry = true;
    
    if (firstEntry) {
        Serial.println("Machine ready - waiting for start button...");
        stepper->disableOutputs();  // Disable stepper to save power
        firstEntry = false;
    }
    
    // Update button states
    startButton.update();
    
    //! ************************************************************************
    //! STEP 1: CHECK FOR START BUTTON PRESS
    //! ************************************************************************
    if (startButton.pressed()) {
        Serial.println("Start button pressed! Beginning cutting cycle...");
        currentState = STATE_CUTTING_CYCLE;
        firstEntry = true;  // Reset for next time
    }
} 