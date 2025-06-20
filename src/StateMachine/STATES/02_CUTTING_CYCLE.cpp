#include "StateMachine.h"

//* ************************************************************************
//* ************************ CUTTING CYCLE ***************************
//* ************************************************************************
//! Cutting cycle state: Continuous movement with speed changes during motion

void cuttingCycleState() {
    static bool firstEntry = true;
    static unsigned long delayStartTime = 0;
    static unsigned long cycleStartTime = 0;  // Track when cycle started
    static long startPosition = 0;
    static bool clampsRetracted = false;
    static bool movementStarted = false;
    static bool clampEngaged = false;
    static bool emergencyStop = false;
    
    // Calculate total distance for entire cutting cycle
    static const long TOTAL_DISTANCE_STEPS = APPROACH_DISTANCE_STEPS + CUTTING_DISTANCE_STEPS + DROPOFF_DISTANCE_STEPS;
    static const long CUTTING_START_POSITION = APPROACH_DISTANCE_STEPS;
    static const long DROPOFF_START_POSITION = APPROACH_DISTANCE_STEPS + CUTTING_DISTANCE_STEPS;
    static const long CLAMP_RETRACT_POSITION = DROPOFF_START_POSITION + CLAMP_RETRACT_DISTANCE_STEPS;
    
    // Emergency stop safety delay (2 seconds after cycle start)
    static const unsigned long EMERGENCY_STOP_DELAY_MS = 500;
    
    if (firstEntry) {
        Serial.println("Cutting cycle started...");
        stepper->enableOutputs();
        currentSubstate = SUBSTATE_APPROACH;
        firstEntry = false;
        clampsRetracted = false;
        movementStarted = false;
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
            //! SUBSTATE 1: APPROACH - EXTEND CLAMPS AND START CONTINUOUS MOVEMENT
            //! ************************************************************************
            
            if (!clampEngaged) {
                extendClamp();
                delayStartTime = millis();
                clampEngaged = true;
                Serial.println("Approach: Clamps extended, waiting 0.5 seconds...");
            }
            
            // Wait 0.5 seconds after clamp engagement
            if (clampEngaged && !movementStarted && (millis() - delayStartTime >= CLAMP_ENGAGE_DELAY_MS)) {
                startPosition = stepper->getCurrentPosition();
                stepper->setSpeedInHz(APPROACH_SPEED);
                stepper->move(TOTAL_DISTANCE_STEPS);  // Start entire movement
                movementStarted = true;
                Serial.println("Approach: Starting continuous movement - 5 inches at speed 5000...");
            }
            
            // Check if approach distance is complete
            if (movementStarted) {
                long currentPosition = stepper->getCurrentPosition();
                long distanceMoved = currentPosition - startPosition;
                
                if (distanceMoved >= CUTTING_START_POSITION) {
                    Serial.println("Approach complete! Changing to cutting speed...");
                    stepper->setSpeedInHz(CUTTING_SPEED);  // Change speed during movement
                    currentSubstate = SUBSTATE_CUTTING;
                    Serial.println("Cutting: Now moving 3 inches at speed 1000...");
                }
            }
            break;
        }
        
        case SUBSTATE_CUTTING:
        {
            //! ************************************************************************
            //! SUBSTATE 2: CUTTING - MONITOR POSITION AND CHANGE TO DROP OFF SPEED
            //! ************************************************************************
            
            long currentPosition = stepper->getCurrentPosition();
            long distanceMoved = currentPosition - startPosition;
            
            // Check if cutting distance is complete
            if (distanceMoved >= DROPOFF_START_POSITION) {
                Serial.println("Cutting complete! Changing to drop off speed...");
                stepper->setSpeedInHz(DROPOFF_SPEED);  // Change speed during movement
                currentSubstate = SUBSTATE_DROPOFF;
                Serial.println("Drop off: Now moving 10 inches at speed 30000...");
            }
            break;
        }
        
        case SUBSTATE_DROPOFF:
        {
            //! ************************************************************************
            //! SUBSTATE 3: DROP OFF - MONITOR FOR CLAMP RETRACTION AT 7 INCHES
            //! ************************************************************************
            
            long currentPosition = stepper->getCurrentPosition();
            long distanceMoved = currentPosition - startPosition;
            
            // Check if we've moved 7 inches into drop off and need to retract clamps
            // Skip clamp retraction if this is an emergency stop
            if (!clampsRetracted && !emergencyStop && distanceMoved >= CLAMP_RETRACT_POSITION) {
                retractClamp();
                clampsRetracted = true;
                Serial.println("Drop off: 7 inches reached - clamps retracted during movement");
            }
            
            // Check if entire movement is complete
            if (!stepper->isRunning()) {
                Serial.println("Drop off complete! Starting return movement...");
                currentSubstate = SUBSTATE_RETURN;
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
                stepper->moveTo(0);  // Return to home offset position
                returnStarted = true;
                Serial.println("Return: Moving to home position at speed 30000...");
            }
            
            // Check if return move is complete
            if (returnStarted && !stepper->isRunning()) {
                if (emergencyStop) {
                    Serial.println("Emergency stop complete! Machine returned to IDLE state safely.");
                    Serial.println("Clamps remain extended as requested.");
                } else {
                    Serial.println("Cutting cycle complete! Returning to IDLE state...");
                }
                currentState = STATE_IDLE;
                
                // Reset all static variables for next cycle
                firstEntry = true;
                returnStarted = false;
                clampsEngaged = false;
                clampsRetracted = false;
                movementStarted = false;
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