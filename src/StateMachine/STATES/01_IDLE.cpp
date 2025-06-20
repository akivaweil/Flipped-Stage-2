#include "StateMachine.h"


//* ************************************************************************
//* ************************ IDLE ***************************
//* ************************************************************************
//! Idle state: Wait for start button press to begin cutting cycle

void idleState() {
    static bool firstEntry = true;
    
    if (firstEntry) {
        retractClamp();
        clampsRetracted = true;
        Serial.println("Machine ready - waiting for start button...");
        firstEntry = false;
    }
    
    // Update button states
    startButton.update();
    homingSwitch.update();
    
    //! ************************************************************************
    //! STEP 1: CHECK FOR HOMING SWITCH PRESS
    //! ************************************************************************
    if (homingSwitch.pressed()) {
        Serial.println("Homing switch pressed! Moving gantry away from home position...");
        stepper->enableOutputs();
        stepper->setDirectionPin(STEPPER_DIR_PIN, HIGH);  // Set direction away from home
        stepper->setSpeedInHz(HOMING_SPEED);
        stepper->move(HOMING_OFFSET_STEPS);
        
        // Wait for movement to complete
        while (stepper->isRunning()) {
            delay(1);
        }
        
        stepper->disableOutputs();  // Disable stepper to save power
        Serial.println("Gantry moved away from home position");
    }
    
    //! ************************************************************************
    //! STEP 2: CHECK FOR START BUTTON PRESS
    //! ************************************************************************
    if (startButton.pressed()) {
        Serial.println("Start button pressed! Beginning cutting cycle...");
        currentState = STATE_CUTTING_CYCLE;
        firstEntry = true;  // Reset for next time
    }
} 