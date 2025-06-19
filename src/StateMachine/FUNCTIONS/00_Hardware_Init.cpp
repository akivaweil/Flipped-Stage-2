#include "StateMachine.h"

//* ************************************************************************
//* ************************ HARDWARE INITIALIZATION *********************
//* ************************************************************************
//! Initialize all hardware components for the Flipped Stage 2 machine
//! @return bool - true if initialization successful, false otherwise

bool initializeHardware() {
    Serial.println("Initializing hardware...");
    
    //! ************************************************************************
    //! STEP 1: INITIALIZE STEPPER MOTOR
    //! ************************************************************************
    stepperEngine.init();
    stepper = stepperEngine.stepperConnectToPin(STEPPER_UL_PIN);
    
    if (stepper) {
        stepper->setDirectionPin(STEPPER_DIR_PIN);
        stepper->setEnablePin(STEPPER_ENABLE_PIN);
        stepper->setAutoEnable(true);
        stepper->setAcceleration(STEPPER_ACCELERATION);
        Serial.println("Stepper motor initialized successfully");
    } else {
        Serial.println("ERROR: Failed to initialize stepper motor!");
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
    
    Serial.println("Input buttons initialized successfully");
    
    //! ************************************************************************
    //! STEP 3: INITIALIZE OUTPUT PINS
    //! ************************************************************************
    
    // Clamp relay control - initialize to extended position
    pinMode(CLAMP_RELAY_PIN, OUTPUT);
    digitalWrite(CLAMP_RELAY_PIN, LOW);  // Start with clamp extended (LOW signal)
    
    Serial.println("Output pins initialized successfully");
    Serial.println("Hardware initialization complete!");
    
    return true;
} 