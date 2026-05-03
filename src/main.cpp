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
bool clampsRetracted = false;

//* ************************************************************************
//* ************************ SETUP FUNCTION ******************************
//* ************************************************************************

void setup() {
    Serial.begin(115200);
    
    //! ************************************************************************
    //! STEP 1: INITIALIZE OTA FUNCTIONALITY
    //! ************************************************************************
    initOTA();
    handleOTA();
    
    //! ************************************************************************
    //! STEP 2: INITIALIZE HARDWARE
    //! ************************************************************************
    if (!initializeHardware()) {
        // Halt system execution if hardware initialization fails
        while(true) {
            delay(1000);
        }
    }
    
    //! ************************************************************************
    //! STEP 3: START STATE MACHINE
    //! ************************************************************************
    // Machine will begin homing sequence automatically
}

//* ************************************************************************
//* ************************ MAIN LOOP ************************************
//* ************************************************************************

void loop() {
    //! ************************************************************************
    //! STEP 1: HANDLE OTA UPDATES (ONLY WHEN IDLE FOR SAFETY)
    //! ************************************************************************
    if (currentState == STATE_IDLE) {
        handleOTA();
    }
    
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
            // Return to homing if unknown state encountered
            currentState = STATE_HOMING;
            break;
    }
    
    //! ************************************************************************
    //! STEP 3: DISPLAY IP PERIODICALLY (FOR OTA)
    //! ************************************************************************
    displayIP();
    
    // Small delay to prevent overwhelming the system
    delay(MAIN_LOOP_DELAY_MS);
}