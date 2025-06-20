#include "StateMachine.h"

//* ************************************************************************
//* ************************ CUTTING CYCLE ***************************
//* ************************************************************************
//! Cutting cycle state: Continuous movement with speed changes during motion

void cuttingCycleState() {
    // Static variables that need to be reset between cycles
    static bool firstEntry = true;
    static unsigned long delayStartTime = 0;
    static unsigned long cycleStartTime = 0;  // Track when cycle started
    static bool clampEngaged = false;
    static bool emergencyStop = false;
    
    // Substate-specific static variables
    static bool approachStarted = false;
    static bool cuttingStarted = false;
    static bool dropoffStarted = false;
    static long dropoffStartPosition = 0;
    static bool dropoffDelayStarted = false;
    static unsigned long dropoffDelayStartTime = 0;
    static bool returnStarted = false;
    static bool clampsEngaged = false;
    
    // Function to reset all static variables for a complete fresh start
    auto resetAllFlags = []() {
        firstEntry = true;
        delayStartTime = 0;
        cycleStartTime = 0;
        clampEngaged = false;
        emergencyStop = false;
        approachStarted = false;
        cuttingStarted = false;
        dropoffStarted = false;
        dropoffStartPosition = 0;
        dropoffDelayStarted = false;
        dropoffDelayStartTime = 0;
        returnStarted = false;
        clampsEngaged = false;
        clampsRetracted = false;
    };
    
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
            
            // CRITICAL: Reset all substate flags to prevent skipping sections in next cycle
            approachStarted = false;
            cuttingStarted = false;
            dropoffStarted = false;
            dropoffDelayStarted = false;
            returnStarted = false;
            clampsEngaged = false;
            
            // Set emergency stop flag and go directly to emergency return
            emergencyStop = true;
            currentSubstate = SUBSTATE_RETURN;
            Serial.println("Emergency return initiated - clamps remain extended");
            Serial.println("All cycle flags reset for clean restart");
        }
    }
    
    switch (currentSubstate) {
        
        case SUBSTATE_APPROACH:
        {
            //! ************************************************************************
            //! SUBSTATE 1: APPROACH - EXTEND CLAMPS AND MOVE APPROACH DISTANCE
            //! ************************************************************************
            
            if (!clampEngaged) {
                extendClamp();
                clampEngaged = true;
                Serial.println("Approach: Clamps extended, starting approach movement...");
            }
            
            // Start approach movement immediately after clamp engagement
            if (clampEngaged && !approachStarted) {
                stepper->setAcceleration(APPROACH_ACCELERATION);
                stepper->setSpeedInHz(APPROACH_SPEED);
                stepper->move(APPROACH_DISTANCE_STEPS);
                approachStarted = true;
                Serial.print("Approach: Moving ");
                Serial.print(APPROACH_DISTANCE_INCHES);
                Serial.print(" inches at speed ");
                Serial.print(APPROACH_SPEED);
                Serial.print(" with acceleration ");
                Serial.print(APPROACH_ACCELERATION);
                Serial.println("...");
            }
            
            // Check if approach movement is complete
            if (approachStarted && !stepper->isRunning()) {
                Serial.println("Approach complete! Starting cutting phase...");
                // Reset acceleration to normal for other movements
                stepper->setAcceleration(STEPPER_ACCELERATION);
                currentSubstate = SUBSTATE_CUTTING;
            }
            break;
        }
        
        case SUBSTATE_CUTTING:
        {
            //! ************************************************************************
            //! SUBSTATE 2: CUTTING - MOVE CUTTING DISTANCE AT SLOW SPEED
            //! ************************************************************************
            
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
            }
            break;
        }
        
        case SUBSTATE_DROPOFF:
        {
            //! ************************************************************************
            //! SUBSTATE 3: DROP OFF - MOVE DROP OFF DISTANCE AND RETRACT CLAMPS
            //! ************************************************************************
            
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
                if (!dropoffDelayStarted) {
                    Serial.println("Drop off complete! Starting 500ms delay before return...");
                    dropoffDelayStartTime = millis();
                    dropoffDelayStarted = true;
                }
                
                // Check if 500ms delay has elapsed
                if (dropoffDelayStarted && (millis() - dropoffDelayStartTime >= 500)) {
                    Serial.println("Drop off delay complete! Starting return movement...");
                    currentSubstate = SUBSTATE_RETURN;
                }
            }
            break;
        }
        
        case SUBSTATE_RETURN:
        {
            //! ************************************************************************
            //! SUBSTATE 4: RETURN - RETRACT CLAMPS AND RETURN TO HOME OFFSET
            //! ************************************************************************
            
            if (!clampsEngaged) {
                retractClamp();
                clampsEngaged = true;
                if (emergencyStop) {
                    Serial.println("Emergency return: Clamps retracted, returning to home...");
                } else {
                    Serial.println("Return: Clamps retracted, returning to home...");
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
                    Serial.println("Clamps retracted for safety.");
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
                
                // CRITICAL: Complete reset of all static variables for next cycle
                Serial.println("Performing complete reset of all cutting cycle flags...");
                resetAllFlags();
                Serial.println("All flags reset - ready for next cycle");
            }
            break;
        }
        
        default:
            Serial.println("ERROR: Unknown cutting substate! Returning to IDLE...");
            currentState = STATE_IDLE;
            resetAllFlags();
            break;
    }
} 