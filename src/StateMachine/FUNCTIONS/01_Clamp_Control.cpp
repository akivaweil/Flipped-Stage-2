#include "StateMachine.h"

//* ************************************************************************
//* ************************ CLAMP CONTROL FUNCTIONS *********************
//* ************************************************************************
//! Functions to control the pneumatic clamp via relay

void extendClamp() {
    //! ************************************************************************
    //! EXTEND CLAMP: ACTIVATE RELAY (HIGH SIGNAL)
    //! ************************************************************************
    digitalWrite(CLAMP_RELAY_PIN, HIGH);
}

void retractClamp() {
    //! ************************************************************************
    //! RETRACT CLAMP: DEACTIVATE RELAY (LOW SIGNAL)
    //! ************************************************************************
    digitalWrite(CLAMP_RELAY_PIN, LOW);
} 