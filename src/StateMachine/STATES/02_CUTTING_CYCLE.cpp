#include "../../../include/StateMachine.h"

//* ************************************************************************
//* ************************ CUTTING CYCLE ***************************
//* ************************************************************************
//! Cutting cycle state: 4 substates for complete cutting operation

void cuttingCycleState() {
    static bool firstEntry = true;
    static unsigned long delayStartTime = 0;
    static long dropoffStartPosition = 0;
    static bool clampsRetracted = false;
    
    if (firstEntry) {
        Serial.println("Cutting cycle started...");
        stepper->enableOutputs();
        currentSubstate = SUBSTATE_APPROACH;
        firstEntry = false;
    }
    
    switch (currentSubstate) {
        
        case SUBSTATE_APPROACH:
        {
            //! ************************************************************************
            //! SUBSTATE 1: APPROACH - EXTEND CLAMPS AND MOVE 5 INCHES
            //! ************************************************************************
            static bool approachStarted = false;
            static bool clampEngaged = false;
            
            if (!clampEngaged) {
                extendClamp();
                delayStartTime = millis();
                clampEngaged = true;
                Serial.println("Approach: Clamps extended, waiting 0.5 seconds...");
            }
            
            // Wait 0.5 seconds after clamp engagement
            if (clampEngaged && !approachStarted && (millis() - delayStartTime >= CLAMP_ENGAGE_DELAY_MS)) {
                stepper->setSpeedInHz(APPROACH_SPEED);
                stepper->move(APPROACH_DISTANCE_STEPS);
                approachStarted = true;
                Serial.println("Approach: Moving 5 inches at speed 5000...");
            }
            
            // Check if approach move is complete
            if (approachStarted && !stepper->isRunning()) {
                Serial.println("Approach complete! Moving to cutting...");
                currentSubstate = SUBSTATE_CUTTING;
                approachStarted = false;
                clampEngaged = false;
            }
            break;
        }
        
        case SUBSTATE_CUTTING:
        {
            //! ************************************************************************
            //! SUBSTATE 2: CUTTING - MOVE 3 INCHES AT SLOW SPEED
            //! ************************************************************************
            static bool cuttingStarted = false;
            
            if (!cuttingStarted) {
                stepper->setSpeedInHz(CUTTING_SPEED);
                stepper->move(CUTTING_DISTANCE_STEPS);
                cuttingStarted = true;
                Serial.println("Cutting: Moving 3 inches at speed 1000...");
            }
            
            // Check if cutting move is complete
            if (!stepper->isRunning()) {
                Serial.println("Cutting complete! Moving to drop off...");
                currentSubstate = SUBSTATE_DROPOFF;
                cuttingStarted = false;
                clampsRetracted = false;
            }
            break;
        }
        
        case SUBSTATE_DROPOFF:
        {
            //! ************************************************************************
            //! SUBSTATE 3: DROP OFF - MOVE 10 INCHES, RETRACT CLAMPS AT 7 INCHES
            //! ************************************************************************
            static bool dropoffStarted = false;
            
            if (!dropoffStarted) {
                dropoffStartPosition = stepper->getCurrentPosition();
                stepper->setSpeedInHz(DROPOFF_SPEED);
                stepper->move(DROPOFF_DISTANCE_STEPS);
                dropoffStarted = true;
                Serial.println("Drop off: Moving 10 inches at speed 30000...");
            }
            
            // Check if we've moved 7 inches and need to retract clamps
            long currentPosition = stepper->getCurrentPosition();
            long distanceMoved = currentPosition - dropoffStartPosition;
            
            if (!clampsRetracted && distanceMoved >= CLAMP_RETRACT_DISTANCE_STEPS) {
                retractClamp();
                clampsRetracted = true;
                Serial.println("Drop off: 7 inches reached - clamps retracted during movement");
            }
            
            // Check if drop off move is complete
            if (!stepper->isRunning()) {
                Serial.println("Drop off complete! Returning to start...");
                currentSubstate = SUBSTATE_RETURN;
                dropoffStarted = false;
            }
            break;
        }
        
        case SUBSTATE_RETURN:
        {
            //! ************************************************************************
            //! SUBSTATE 4: RETURN - ENGAGE CLAMPS AND RETURN TO HOME OFFSET
            //! ************************************************************************
            static bool returnStarted = false;
            static bool clampsEngaged = false;
            
            if (!clampsEngaged) {
                extendClamp();
                clampsEngaged = true;
                Serial.println("Return: Clamps engaged, returning to home...");
            }
            
            if (clampsEngaged && !returnStarted) {
                stepper->setSpeedInHz(RETURN_SPEED);
                stepper->moveTo(0);  // Return to home offset position
                returnStarted = true;
                Serial.println("Return: Moving to home position at speed 30000...");
            }
            
            // Check if return move is complete
            if (returnStarted && !stepper->isRunning()) {
                Serial.println("Cutting cycle complete! Returning to IDLE state...");
                currentState = STATE_IDLE;
                
                // Reset all static variables for next cycle
                firstEntry = true;
                returnStarted = false;
                clampsEngaged = false;
                clampsRetracted = false;
            }
            break;
        }
        
        default:
            Serial.println("ERROR: Unknown cutting substate! Returning to IDLE...");
            currentState = STATE_IDLE;
            firstEntry = true;
            break;
    }
} 