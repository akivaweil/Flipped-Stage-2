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
        Serial.print("Machine ready - waiting for start button... Current position: ");
        Serial.println(stepper->getCurrentPosition());
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
        
        // Move to proper offset position
        stepper->setDirectionPin(STEPPER_DIR_PIN, HIGH);  // Set direction away from home
        stepper->setSpeedInHz(HOMING_SPEED);
        stepper->move(HOMING_OFFSET_STEPS);
        
        // Wait for movement to complete
        while (stepper->isRunning()) {
            delay(1);
        }
        
        // Set final position to offset value
        stepper->setCurrentPosition(HOMING_OFFSET_STEPS);
        Serial.print("Position corrected! New position: ");
        Serial.println(stepper->getCurrentPosition());
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