#include "StateMachine.h"

static constexpr unsigned long DROPOFF_WAIT_MS = 50;

//* ************************************************************************
//* ************************ CUTTING CYCLE ***************************
//* ************************************************************************
//! Cutting cycle state: Sequential movement through approach, cutting, dropoff, and return phases
//! Uses absolute positioning to move through predetermined positions along the linear axis

void cuttingCycleState() {
    static bool firstEntry = true;
    static unsigned long cycleStartTime = 0;
    static unsigned long dropoffDelayStartTime = 0;
    static bool emergencyStop = false;
    static bool earlyClampReleased = false;
    static long dropoffTargetSteps = DROPOFF_POSITION_STEPS;
    static bool dropoffTargetPicked = false;

    if (firstEntry) {
        currentSubstate = SUBSTATE_APPROACH;
        firstEntry = false;
        emergencyStop = false;
        earlyClampReleased = false;
        dropoffTargetPicked = false;
        cycleStartTime = millis();
        //! ************************************************************************
        //! TURN ON WARNING LIGHT AT START OF CUTTING CYCLE
        //! ************************************************************************
        turnOnWarningLight();
    }
    
    //! ************************************************************************
    //! EMERGENCY STOP CHECK - START BUTTON PRESSED DURING CYCLE
    //! ************************************************************************
    //! Monitor for emergency stop condition after safety delay period
    //! If start button pressed during cycle, immediately stop all movement and return home
    if (!emergencyStop && (millis() - cycleStartTime > EMERGENCY_STOP_DELAY_MS)) {
        startButton.update();
        if (startButton.pressed()) {
            stepper->forceStop();
            extendClamp();
            emergencyStop = true;
            currentSubstate = SUBSTATE_RETURN;
            //! ************************************************************************
            //! TURN OFF WARNING LIGHT ON EMERGENCY STOP
            //! ************************************************************************
            turnOffWarningLight();
        }
    }
    
    switch (currentSubstate) {
        
        case SUBSTATE_APPROACH:
        {
            //! ************************************************************************
            //! SUBSTATE 1: APPROACH - EXTEND CLAMPS AND MOVE TO APPROACH POSITION
            //! ************************************************************************
            //! Extend clamps to secure workpiece, then move 3.5 inches forward from home offset
            //! Position: 0 → 3.5 inches (APPROACH_POSITION_STEPS absolute position)
            //! Speed: APPROACH_SPEED with reduced acceleration for precision
            extendClamp();
            stepper->setAcceleration(APPROACH_ACCELERATION);
            stepper->setSpeedInHz(APPROACH_SPEED);
            stepper->moveTo(APPROACH_POSITION_STEPS);
            
            if (!stepper->isRunning()) {
                stepper->setAcceleration(STEPPER_ACCELERATION);
                currentSubstate = SUBSTATE_CUTTING;
            }
            break;
        }
        
        case SUBSTATE_CUTTING:
        {
            //! ************************************************************************
            //! SUBSTATE 2: CUTTING - MOVE THROUGH CUTTING ZONE AT SLOW SPEED
            //! ************************************************************************
            //! Move through cutting zone at slow speed for precise cutting operation
            //! Position: 3.5 inches → 9.5 inches (CUTTING_POSITION_STEPS absolute position)
            //! Speed: CUTTING_SPEED (slow for precision cutting)
            stepper->setSpeedInHz(CUTTING_SPEED);
            stepper->moveTo(CUTTING_POSITION_STEPS);
            
            if (!stepper->isRunning()) {
                currentSubstate = SUBSTATE_DROPOFF;
            }
            break;
        }
        
        case SUBSTATE_DROPOFF:
        {
            //! ************************************************************************
            //! SUBSTATE 3: DROPOFF - MOVE TO DROPOFF POSITION WITH EARLY CLAMP RELEASE
            //! ************************************************************************
            //! Move to a randomized dropoff position within the last DROPOFF_RANDOM_RANGE_INCHES
            //! Position: cutting → random in [DROPOFF_MIN_POSITION_STEPS, DROPOFF_POSITION_STEPS]
            //! Speed: DROPOFF_SPEED (fast movement to dropoff zone)
            //! Early clamp release: Retract clamps when motor passes EARLY_CLAMP_RELEASE position
            //! After movement complete: wait DROPOFF_WAIT_MS before return
            if (!dropoffTargetPicked) {
                dropoffTargetSteps = random(DROPOFF_MIN_POSITION_STEPS, DROPOFF_POSITION_STEPS + 1);
                dropoffTargetPicked = true;
            }
            stepper->setSpeedInHz(DROPOFF_SPEED);
            stepper->moveTo(dropoffTargetSteps);
            
            //! ************************************************************************
            //! EARLY CLAMP RELEASE - RETRACT CLAMPS AT 12 INCHES WITHOUT STOPPING MOTOR
            //! ************************************************************************
            //! Check if motor has passed the early clamp release position (12 inches)
            //! Release clamps during movement to allow workpiece to drop early
            if (!earlyClampReleased && !emergencyStop && 
                stepper->getCurrentPosition() >= EARLY_CLAMP_RELEASE_POSITION_STEPS) {
                retractClamp();
                earlyClampReleased = true;
            }
            
            if (!stepper->isRunning()) {
                // Ensure clamps are retracted if not already done during early release
                if (!emergencyStop && !earlyClampReleased) {
                    retractClamp();
                }
                
                if (dropoffDelayStartTime == 0) {
                    dropoffDelayStartTime = millis();
                }
                
                if (millis() - dropoffDelayStartTime >= DROPOFF_WAIT_MS) {
                    currentSubstate = SUBSTATE_RETURN;
                }
            }
            break;
        }
        
        case SUBSTATE_RETURN:
        {
            //! ************************************************************************
            //! SUBSTATE 4: RETURN - RETURN TO ZERO POSITION
            //! ************************************************************************
            //! Normal return: clamps already retracted, return at RETURN_SPEED
            //! Emergency stop return: keep clamps extended, return at HOMING_SPEED,
            //!   then retract clamps once home position is reached
            //! Position: Current position → 0 inches (HOME_POSITION_STEPS absolute position)
            //! Upon completion: return to IDLE state and reset all cycle variables
            if (emergencyStop) {
                stepper->setSpeedInHz(HOMING_SPEED);
            } else {
                retractClamp();
                stepper->setSpeedInHz(RETURN_SPEED);
            }
            stepper->moveTo(HOME_POSITION_STEPS);
            
            if (!stepper->isRunning()) {
                if (emergencyStop) {
                    retractClamp();
                }
                currentState = STATE_IDLE;
                firstEntry = true;
                dropoffDelayStartTime = 0;
                emergencyStop = false;
                earlyClampReleased = false;
                //! ************************************************************************
                //! TURN OFF WARNING LIGHT AT END OF CUTTING CYCLE
                //! ************************************************************************
                turnOffWarningLight();
            }
            break;
        }
    }
} 