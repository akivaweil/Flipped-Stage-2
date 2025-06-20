#include "StateMachine.h"

//* ************************************************************************
//* ************************ CUTTING CYCLE ***************************
//* ************************************************************************
//! Cutting cycle state: Continuous movement with speed changes during motion

void cuttingCycleState() {
    static bool firstEntry = true;
    static unsigned long delayStartTime = 0;
    static unsigned long cycleStartTime = 0;  // Track when cycle started
    static bool clampEngaged = false;
    static bool emergencyStop = false;
    
    if (firstEntry) {
        Serial.println("Cutting cycle started...");
        
        // Check for closed-loop position drift before starting cycle
        long currentPos = stepper->getCurrentPosition();
        long expectedPos = HOMING_OFFSET_STEPS;
        long positionError = abs(currentPos - expectedPos);
        
        if (positionError > 10) {  // Allow small tolerance
            Serial.print("WARNING: Large position error detected! Current: ");
            Serial.print(currentPos);
            Serial.print(", Expected: ");
            Serial.print(expectedPos);
            Serial.print(", Error: ");
            Serial.println(positionError);
            Serial.println("This suggests closed-loop stepper corrections occurred");
        }
        
        currentSubstate = SUBSTATE_APPROACH;
        firstEntry = false;
        clampsRetracted = false;
        clampEngaged = false;
        emergencyStop = false;
        cycleStartTime = millis();  // Record cycle start time
    }
    
    //! ************************************************************************
    //! EMERGENCY STOP CHECK - START BUTTON PRESSED DURING CYCLE
    //! ************************************************************************
    // Only check for emergency stop after safety delay and if not already in emergency mode
    if (!emergencyStop && (millis() - cycleStartTime > EMERGENCY_STOP_DELAY_MS)) {
        startButton.update();
        if (startButton.pressed()) {
            Serial.println("EMERGENCY STOP! Start button pressed during cutting cycle");
            Serial.println("Stopping all movement and returning home...");
            
            // Immediately stop stepper movement
            stepper->forceStop();
            
            // Ensure clamps stay extended as requested
            extendClamp();
            
            // Set emergency stop flag and go directly to emergency return
            emergencyStop = true;
            currentSubstate = SUBSTATE_RETURN;
            Serial.println("Emergency return initiated - clamps remain extended");
        }
    }
    
    switch (currentSubstate) {
        
        case SUBSTATE_APPROACH:
        {
            //! ************************************************************************
            //! SUBSTATE 1: APPROACH - EXTEND CLAMPS AND MOVE APPROACH DISTANCE
            //! ************************************************************************
            static bool approachStarted = false;
            
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
                Serial.print("Approach: Moving ");
                Serial.print(APPROACH_DISTANCE_INCHES);
                Serial.print(" inches at speed ");
                Serial.print(APPROACH_SPEED);
                Serial.println("...");
            }
            
            // Check if approach movement is complete
            if (approachStarted && !stepper->isRunning()) {
                Serial.println("Approach complete! Starting cutting phase...");
                currentSubstate = SUBSTATE_CUTTING;
                approachStarted = false; // Reset for next cycle
            }
            break;
        }
        
        case SUBSTATE_CUTTING:
        {
            //! ************************************************************************
            //! SUBSTATE 2: CUTTING - MOVE CUTTING DISTANCE AT SLOW SPEED
            //! ************************************************************************
            static bool cuttingStarted = false;
            
            if (!cuttingStarted) {
                stepper->setSpeedInHz(CUTTING_SPEED);
                stepper->move(CUTTING_DISTANCE_STEPS);
                cuttingStarted = true;
                Serial.print("Cutting: Moving ");
                Serial.print(CUTTING_DISTANCE_INCHES);
                Serial.print(" inches at speed ");
                Serial.print(CUTTING_SPEED);
                Serial.println("...");
            }
            
            // Check if cutting movement is complete
            if (cuttingStarted && !stepper->isRunning()) {
                Serial.println("Cutting complete! Starting drop off phase...");
                currentSubstate = SUBSTATE_DROPOFF;
                cuttingStarted = false; // Reset for next cycle
            }
            break;
        }
        
        case SUBSTATE_DROPOFF:
        {
            //! ************************************************************************
            //! SUBSTATE 3: DROP OFF - MOVE DROP OFF DISTANCE AND RETRACT CLAMPS
            //! ************************************************************************
            static bool dropoffStarted = false;
            static long dropoffStartPosition = 0;
            
            if (!dropoffStarted) {
                dropoffStartPosition = stepper->getCurrentPosition();
                stepper->setSpeedInHz(DROPOFF_SPEED);
                stepper->move(DROPOFF_DISTANCE_STEPS);
                dropoffStarted = true;
                Serial.print("Drop off: Moving ");
                Serial.print(DROPOFF_DISTANCE_INCHES);
                Serial.print(" inches at speed ");
                Serial.print(DROPOFF_SPEED);
                Serial.println("...");
            }
            
            // Check if we've moved the clamp retract distance and need to retract clamps
            // Skip clamp retraction if this is an emergency stop
            if (!clampsRetracted && !emergencyStop && dropoffStarted) {
                long currentPosition = stepper->getCurrentPosition();
                long distanceMoved = currentPosition - dropoffStartPosition;
                
                if (distanceMoved >= CLAMP_RETRACT_DISTANCE_STEPS) {
                    retractClamp();
                    clampsRetracted = true;
                    Serial.print("Drop off: ");
                    Serial.print(CLAMP_RETRACT_DISTANCE_INCHES);
                    Serial.println(" inches reached - clamps retracted during movement");
                }
            }
            
            // Check if drop off movement is complete
            if (dropoffStarted && !stepper->isRunning()) {
                Serial.println("Drop off complete! Starting return movement...");
                currentSubstate = SUBSTATE_RETURN;
                dropoffStarted = false; // Reset for next cycle
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
                if (emergencyStop) {
                    Serial.println("Emergency return: Clamps secured, returning to home...");
                } else {
                    Serial.println("Return: Clamps engaged, returning to home...");
                }
            }
            
            if (clampsEngaged && !returnStarted) {
                stepper->setSpeedInHz(RETURN_SPEED);
                stepper->moveTo(HOMING_OFFSET_STEPS);  // Return to home offset position
                returnStarted = true;
                Serial.print("Return: Moving to home offset position at speed ");
                Serial.print(RETURN_SPEED);
                Serial.println("...");
            }
            
            // Check if return move is complete
            if (returnStarted && !stepper->isRunning()) {
                // Validate final position
                long finalPosition = stepper->getCurrentPosition();
                if (emergencyStop) {
                    Serial.println("Emergency stop complete! Machine returned to IDLE state safely.");
                    Serial.println("Clamps remain extended as requested.");
                } else {
                    Serial.println("Cutting cycle complete! Returning to IDLE state...");
                }
                Serial.print("Final position: ");
                Serial.print(finalPosition);
                Serial.print(" (Target: ");
                Serial.print(HOMING_OFFSET_STEPS);
                Serial.println(")");
                
                // Position validation - warn if drift detected
                if (abs(finalPosition - HOMING_OFFSET_STEPS) > 5) {
                    Serial.println("WARNING: Position drift detected! Consider recalibration.");
                }
                
                currentState = STATE_IDLE;
                
                // Reset all static variables for next cycle
                firstEntry = true;
                returnStarted = false;
                clampsEngaged = false;
                clampsRetracted = false;
                clampEngaged = false;
                emergencyStop = false;
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