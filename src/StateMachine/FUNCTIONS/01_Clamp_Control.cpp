#include "StateMachine.h"

//* ************************************************************************
//* ************************ CLAMP CONTROL FUNCTIONS *********************
//* ************************************************************************
//! Functions to control the pneumatic clamp via relay

void extendClamp() {
    //! ************************************************************************
    //! EXTEND CLAMP: ACTIVATE RELAY (LOW SIGNAL)
    //! ************************************************************************
    digitalWrite(CLAMP_RELAY_PIN, LOW);
    Serial.println("Clamp extended");
}

void retractClamp() {
    //! ************************************************************************
    //! RETRACT CLAMP: DEACTIVATE RELAY (HIGH SIGNAL)
    //! ************************************************************************
    digitalWrite(CLAMP_RELAY_PIN, HIGH);
    Serial.println("Clamp retracted");
} 