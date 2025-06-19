#include "StateMachine.h"

//* ************************************************************************
//* ************************ HOMING ***************************
//* ************************************************************************
//! Homing state: Move in negative direction until homing switch triggers,
//! then offset 0.2 inches and transition to IDLE state

void homingState() {
    static bool homingStarted = false;
    static bool homingComplete = false;
    
    if (!homingStarted) {
        Serial.println("Starting homing sequence...");
        
        //! ************************************************************************
        //! STEP 1: ENABLE STEPPER AND SET DIRECTION
        //! ************************************************************************
        stepper->setDirectionPin(STEPPER_DIR_PIN);
        stepper->enableOutputs();
        stepper->setSpeedInHz(HOMING_SPEED);
        
        // Set direction negative (assuming LOW = negative direction)
        digitalWrite(STEPPER_DIR_PIN, LOW);
        
        //! ************************************************************************
        //! STEP 2: START MOVING TOWARDS HOME SWITCH
        //! ************************************************************************
        stepper->runForward();
        homingStarted = true;
        Serial.println("Moving towards home switch...");
    }
    
    // Update button states
    homingSwitch.update();
    
    //! ************************************************************************
    //! STEP 3: CHECK FOR HOMING SWITCH ACTIVATION
    //! ************************************************************************
    if (homingSwitch.read() && !homingComplete) {
        Serial.println("Homing switch triggered! Stopping motor...");
        stepper->forceStopAndNewPosition(0);  // Stop and set current position as 0
        
        //! ************************************************************************
        //! STEP 4: MOVE OFFSET DISTANCE FROM HOME SWITCH
        //! ************************************************************************
        stepper->setCurrentPosition(0);
        stepper->moveTo(HOMING_OFFSET_STEPS);
        homingComplete = true;
        Serial.println("Moving to offset position...");
    }
    
    //! ************************************************************************
    //! STEP 5: CHECK IF OFFSET MOVE IS COMPLETE
    //! ************************************************************************
    if (homingComplete && !stepper->isRunning()) {
        Serial.println("Homing complete! Transitioning to IDLE state.");
        stepper->setCurrentPosition(0);  // Reset position counter
        currentState = STATE_IDLE;
        
        // Reset homing flags for next time
        homingStarted = false;
        homingComplete = false;
    }
} 