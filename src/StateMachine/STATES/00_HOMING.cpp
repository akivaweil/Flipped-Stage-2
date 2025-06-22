#include "StateMachine.h"

//* ************************************************************************
//* ************************ HOMING ***************************
//* ************************************************************************
//! Homing state: Move in negative direction until homing switch triggers,
//! then offset and transition to IDLE state

void homingState() {
    static bool homingStarted = false;
    static bool homingComplete = false;
    static unsigned long homingTimer = 0;
    
    // Handle OTA after 5 seconds to allow uploads even without USB connection
    if (millis() - homingTimer >= 2000) {
        handleOTA();
        homingTimer = millis(); // Reset timer
    }
    
    if (!homingStarted) {
        //! ************************************************************************
        //! STEP 1: RETRACT CLAMPS FOR SAFETY DURING HOMING
        //! ************************************************************************
        retractClamp();
        clampsRetracted = true;
        
        //! ************************************************************************
        //! STEP 2: ENABLE STEPPER AND SET DIRECTION
        //! ************************************************************************
        stepper->setDirectionPin(STEPPER_DIR_PIN);
        stepper->enableOutputs();
        stepper->setSpeedInHz(HOMING_SPEED);
        
        //! ************************************************************************
        //! STEP 3: START MOVING TOWARDS HOME SWITCH (NEGATIVE DIRECTION)
        //! ************************************************************************
        stepper->runBackward();
        homingStarted = true;
        homingTimer = millis(); // Initialize timer when homing starts
    }
    
    // Update button states
    homingSwitch.update();
    
    //! ************************************************************************
    //! STEP 4: CHECK FOR HOMING SWITCH ACTIVATION
    //! ************************************************************************
    if (homingSwitchActive() && !homingComplete) {
        stepper->forceStopAndNewPosition(0);  // Stop and set current position as 0
        
        //! ************************************************************************
        //! STEP 5: MOVE OFFSET DISTANCE FROM HOME SWITCH
        //! ************************************************************************
        stepper->setCurrentPosition(0);
        stepper->moveTo(HOMING_OFFSET_STEPS);
        homingComplete = true;
    }
    
    //! ************************************************************************
    //! STEP 6: CHECK IF OFFSET MOVE IS COMPLETE
    //! ************************************************************************
    if (homingComplete && !stepper->isRunning()) {
        stepper->setCurrentPosition(HOMING_OFFSET_STEPS);  // Set position to homing offset
        currentState = STATE_IDLE;
        
        // Reset homing flags for next time
        homingStarted = false;
        homingComplete = false;
    }
} 