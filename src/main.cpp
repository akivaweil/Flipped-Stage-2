//* ************************************************************************
//* ************************ FLIPPED STAGE 2 MAIN ***********************
//* ************************************************************************
//! Main application for Flipped Stage 2 linear cutting machine
//! Features: OTA updates, state machine control, stepper motor, clamp control

#include <Arduino.h>
#include "OTA_Manager.h"
#include "StateMachine.h"

//* ************************************************************************
//* ************************ GLOBAL VARIABLES ****************************
//* ************************************************************************

// State machine variables
MachineState currentState = STATE_HOMING;
CuttingSubstate currentSubstate = SUBSTATE_APPROACH;

// Hardware objects
FastAccelStepperEngine stepperEngine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;
Bounce2::Button startButton = Bounce2::Button();
Bounce2::Button homingSwitch = Bounce2::Button();

//* ************************************************************************
//* ************************ SETUP FUNCTION ******************************
//* ************************************************************************

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Flipped Stage 2 Machine Starting ===");
    
    //! ************************************************************************
    //! STEP 1: INITIALIZE OTA FUNCTIONALITY
    //! ************************************************************************
    initOTA();
    
    //! ************************************************************************
    //! STEP 2: INITIALIZE HARDWARE
    //! ************************************************************************
    if (!initializeHardware()) {
        Serial.println("CRITICAL ERROR: Hardware initialization failed!");
        Serial.println("System halted - check connections and restart");
        while(true) {
            delay(1000);  // Halt system execution
        }
    }
    
    //! ************************************************************************
    //! STEP 3: START STATE MACHINE
    //! ************************************************************************
    Serial.println("Starting state machine...");
    Serial.println("Machine will begin homing sequence");
    
    Serial.println("=== Setup Complete ===\n");
}

//* ************************************************************************
//* ************************ MAIN LOOP ************************************
//* ************************************************************************

void loop() {
    //! ************************************************************************
    //! STEP 1: HANDLE OTA UPDATES
    //! ************************************************************************
    handleOTA();
    
    //! ************************************************************************
    //! STEP 2: RUN STATE MACHINE
    //! ************************************************************************
    switch (currentState) {
        case STATE_HOMING:
            homingState();
            break;
            
        case STATE_IDLE:
            idleState();
            break;
            
        case STATE_CUTTING_CYCLE:
            cuttingCycleState();
            break;
            
        default:
            Serial.println("ERROR: Unknown state! Returning to homing...");
            currentState = STATE_HOMING;
            break;
    }
    
    //! ************************************************************************
    //! STEP 3: DISPLAY IP PERIODICALLY (FOR OTA)
    //! ************************************************************************
    displayIP();
    
    // Small delay to prevent overwhelming the serial output
    delay(MAIN_LOOP_DELAY_MS);
}