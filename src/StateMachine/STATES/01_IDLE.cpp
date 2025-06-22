#include "StateMachine.h"


//* ************************************************************************
//* ************************ IDLE ***************************
//* ************************************************************************
//! Idle state: Wait for start button press to begin cutting cycle

void idleState() {
    static bool firstEntry = true;
    
    if (firstEntry) {
        // Ensure complete reset when entering IDLE state
        retractClamp();
        clampsRetracted = true;
        
        // Reset cutting substate to default for clean next cycle start
        currentSubstate = SUBSTATE_APPROACH;
        firstEntry = false;
    }
    
    // Update button states
    startButton.update();
    homingSwitch.update();
    
    //! ************************************************************************
    //! STEP 1: CHECK FOR HOMING SWITCH PRESS - CLOSED LOOP POSITION CORRECTION
    //! ************************************************************************
    if (homingSwitch.pressed()) {
        // Force position correction - set current position to 0 since we're at home switch
        stepper->setCurrentPosition(0);
    }
    
    //! ************************************************************************
    //! STEP 2: CHECK FOR START BUTTON PRESS
    //! ************************************************************************
    if (startButton.pressed()) {
        currentState = STATE_CUTTING_CYCLE;
        firstEntry = true;  // Reset for next time
    }
} 