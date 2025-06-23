#include "StateMachine.h"

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
    
    if (firstEntry) {
        currentSubstate = SUBSTATE_APPROACH;
        firstEntry = false;
        emergencyStop = false;
        earlyClampReleased = false;
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
            //! Move to final dropoff position with early clamp release at 12 inches
            //! Position: 9.5 inches → 24.5 inches (DROPOFF_POSITION_STEPS absolute position)
            //! Speed: DROPOFF_SPEED (fast movement to dropoff zone)
            //! Early clamp release: Retract clamps when motor passes 12 inch position
            //! After movement complete: wait 500ms before return
            stepper->setSpeedInHz(DROPOFF_SPEED);
            stepper->moveTo(DROPOFF_POSITION_STEPS);
            
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
                
                if (millis() - dropoffDelayStartTime >= 200) {
                    currentSubstate = SUBSTATE_RETURN;
                }
            }
            break;
        }
        
        case SUBSTATE_RETURN:
        {
            //! ************************************************************************
            //! SUBSTATE 4: RETURN - RETRACT CLAMPS AND RETURN TO HOME OFFSET POSITION
            //! ************************************************************************
            //! Ensure clamps are retracted for safety, then return to home offset position
            //! Position: 24.5 inches → 0 inches (HOME_POSITION_STEPS absolute position)
            //! Speed: RETURN_SPEED (fast return movement)
            //! Upon completion: Check if emergency stop occurred for recovery sequence
            retractClamp();
            stepper->setSpeedInHz(RETURN_SPEED);
            stepper->moveTo(HOME_POSITION_STEPS);
            
            if (!stepper->isRunning()) {
                if (emergencyStop) {
                    //! ************************************************************************
                    //! EMERGENCY STOP RECOVERY: TRANSITION TO EMERGENCY RECOVERY SUBSTATE
                    //! ************************************************************************
                    currentSubstate = SUBSTATE_EMERGENCY_RECOVERY;
                } else {
                    //! ************************************************************************
                    //! NORMAL CYCLE COMPLETION: RETURN TO IDLE STATE
                    //! ************************************************************************
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
            }
            break;
        }
        
        case SUBSTATE_EMERGENCY_RECOVERY:
        {
            //! ************************************************************************
            //! SUBSTATE 5: EMERGENCY RECOVERY - MOVE 1 INCH AWAY FROM HOME THEN RE-HOME
            //! ************************************************************************
            //! After emergency stop and return to home, move 1 inch away from home position
            //! then transition to homing state to re-establish accurate home position
            //! Position: 0 inches → -1 inch (EMERGENCY_RECOVERY_POSITION_STEPS)
            //! Speed: RETURN_SPEED (fast movement away from home)
            //! Upon completion: transition to HOMING state
            stepper->setSpeedInHz(RETURN_SPEED);
            stepper->moveTo(EMERGENCY_RECOVERY_POSITION_STEPS);
            
            if (!stepper->isRunning()) {
                //! ************************************************************************
                //! TRANSITION TO HOMING STATE FOR RE-HOMING SEQUENCE
                //! ************************************************************************
                currentState = STATE_HOMING;
                firstEntry = true;
                dropoffDelayStartTime = 0;
                emergencyStop = false;
                earlyClampReleased = false;
                //! ************************************************************************
                //! TURN OFF WARNING LIGHT BEFORE HOMING
                //! ************************************************************************
                turnOffWarningLight();
            }
            break;
        }
    }
} 