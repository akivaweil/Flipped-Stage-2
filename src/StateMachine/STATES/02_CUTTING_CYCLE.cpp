#include "../../../include/StateMachine.h"

//* ************************************************************************
//* ************************ CUTTING CYCLE ***************************
//* ************************************************************************
//! Cutting cycle state: Placeholder for 4 substates to be defined later

void cuttingCycleState() {
    static bool firstEntry = true;
    
    if (firstEntry) {
        Serial.println("Cutting cycle started...");
        stepper->enableOutputs();
        firstEntry = false;
    }
    
    //! ************************************************************************
    //! PLACEHOLDER: CUTTING CYCLE SUBSTATES TO BE IMPLEMENTED
    //! ************************************************************************
    //! This will contain 4 substates for the cutting process:
    //! - Substate 1: TBD
    //! - Substate 2: TBD  
    //! - Substate 3: TBD
    //! - Substate 4: TBD
    
    // Temporary: Return to IDLE after 5 seconds for testing
    static unsigned long cycleStartTime = millis();
    if (millis() - cycleStartTime > 5000) {
        Serial.println("Cutting cycle complete! Returning to IDLE...");
        currentState = STATE_IDLE;
        firstEntry = true;
        cycleStartTime = millis();  // Reset timer
    }
} 