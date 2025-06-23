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

//* ************************************************************************
//* ************************ WARNING LIGHT CONTROL FUNCTIONS *************
//* ************************************************************************
//! Functions to control the warning light via relay (Active LOW)

void turnOnWarningLight() {
    //! ************************************************************************
    //! TURN ON WARNING LIGHT: ACTIVATE RELAY (LOW SIGNAL - ACTIVE LOW)
    //! ************************************************************************
    digitalWrite(WARNING_LIGHT_PIN, LOW);
}

void turnOffWarningLight() {
    //! ************************************************************************
    //! TURN OFF WARNING LIGHT: DEACTIVATE RELAY (HIGH SIGNAL - ACTIVE LOW)
    //! ************************************************************************
    digitalWrite(WARNING_LIGHT_PIN, HIGH);
} 