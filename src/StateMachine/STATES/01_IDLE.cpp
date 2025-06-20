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
        
        Serial.print("Machine ready - waiting for start button... Current position: ");
        Serial.println(stepper->getCurrentPosition());
        Serial.println("All systems reset and ready for next cutting cycle");
        firstEntry = false;
    }
    
    // Update button states
    startButton.update();
    homingSwitch.update();
    
    //! ************************************************************************
    //! STEP 1: CHECK FOR HOMING SWITCH PRESS - CLOSED LOOP POSITION CORRECTION
    //! ************************************************************************
    if (homingSwitch.pressed()) {
        Serial.println("POSITION DRIFT DETECTED! Homing switch active but ESP32 thinks position is:");
        Serial.println(stepper->getCurrentPosition());
        Serial.println("Correcting closed-loop stepper position mismatch...");
        
        // Force position correction - set current position to 0 since we're at home switch
        stepper->setCurrentPosition(0);
        Serial.println("Position reset to 0 (at home switch)");
        Serial.println("Position corrected! Machine remains at home switch position.");
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