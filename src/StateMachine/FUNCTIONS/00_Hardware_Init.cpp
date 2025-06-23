#include "StateMachine.h"

//* ************************************************************************
//* ************************ HARDWARE INITIALIZATION *********************
//* ************************************************************************
//! Initialize all hardware components for the Flipped Stage 2 machine
//! @return bool - true if initialization successful, false otherwise

bool initializeHardware() {
    //! ************************************************************************
    //! STEP 1: INITIALIZE STEPPER MOTOR
    //! ************************************************************************
    stepperEngine.init();
    stepper = stepperEngine.stepperConnectToPin(STEPPER_PUL_PIN);
    
    if (stepper) {
        stepper->setDirectionPin(STEPPER_DIR_PIN);
        stepper->setEnablePin(STEPPER_ENABLE_PIN);
        stepper->setAutoEnable(false);  // Manual control of enable pin to maintain holding torque
        stepper->setAcceleration(STEPPER_ACCELERATION);
        stepper->enableOutputs();  // Keep stepper enabled to maintain position
    } else {
        return false;
    }
    
    //! ************************************************************************
    //! STEP 2: INITIALIZE INPUT BUTTONS WITH DEBOUNCING
    //! ************************************************************************
    
    // Start button (Active HIGH with pulldown)
    startButton.attach(START_BUTTON_PIN, INPUT_PULLDOWN);
    startButton.interval(START_BUTTON_DEBOUNCE_MS);
    startButton.setPressedState(HIGH);
    
    // Homing switch (Active HIGH with pulldown) 
    homingSwitch.attach(HOMING_SWITCH_PIN, INPUT_PULLDOWN);
    homingSwitch.interval(HOMING_SWITCH_DEBOUNCE_MS);
    homingSwitch.setPressedState(HIGH);
    
    //! ************************************************************************
    //! STEP 3: INITIALIZE OUTPUT PINS
    //! ************************************************************************
    
    // Clamp relay control - initialize to retracted position
    pinMode(CLAMP_RELAY_PIN, OUTPUT);
    digitalWrite(CLAMP_RELAY_PIN, LOW);  // Start with clamp retracted (LOW signal)
    
    // Warning light relay control - initialize to OFF position (Active LOW relay)
    pinMode(WARNING_LIGHT_PIN, OUTPUT);
    digitalWrite(WARNING_LIGHT_PIN, HIGH);  // Start with warning light OFF (HIGH signal for active LOW relay)
    
    return true;
} 