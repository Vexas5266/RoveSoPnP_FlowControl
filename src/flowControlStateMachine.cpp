#include "flowControl.hpp"

void FlowControl::tickStateMachine()
{
    switch (m_current_state)
    {
        // ==========================================
        // SYSTEM STATES
        // ==========================================
        case FlowControlState::IDLE:
            // Waiting for commands from the UI (Homing, Calibration, or Start Job)
            break;

        case FlowControlState::RUNNING:
            // Actively executing a homing, calibration, or pick/place sequence.
            break;

        case FlowControlState::JOB_FAILED:
            // The job has failed. Clean up, turn off peripherals, return to IDLE.
            break;

        // ==========================================
        // VISION CALIBRATION SEQUENCE
        // ==========================================
        case FlowControlState::BOARD_DETECT_SAFE_START_STATE:
            // This is just a start state marker for the vision calibration sequence. It allows the UI to jump directly to this point for testing without
            // running the homing sequence first. It also serves as a point for the state machine to resume from if the machine is paused or interrupted
            // during the vision calibration steps. The state machine can return to this known safe state and re-run the vision calibration without needing
            // to redo the homing sequence.

            // Do any cleanup here.

            // Transition to CALIBRATION_HOME.
            break;

        case FlowControlState::CALIBRATION_HOME:
            // Execute the homing sequence to find the machine's XYZ zero reference.
            // GANTRY.HOME();
            // HEAD.HOME();
            // Transition to CALIBRATION_HOMING.
            break;

        case FlowControlState::CALIBRATION_HOMING:
            // IF (!GRBL.isBusy()) THEN
            //     Transition to MOVE_Z_SAFE_CALIBRATION.
            // ENDIF
            break;

        case FlowControlState::MOVE_Z_SAFE_CALIBRATION:
            // Move the Z-axis to a safe height for XY travel during calibration.

            // IF (!GRBL.isBusy()) THEN
            //     HEAD.SETHEIGHT(SAFE_Z_HEIGHT);
            //     Transition to MOVE_TO_FIDUCIAL_1.
            // ENDIF
            break;

        case FlowControlState::MOVE_TO_FIDUCIAL_1:
            // In this state the user will be prompted to manually jog the gantry over the first fiducial and then press a button in the UI to confirm they are in
            // position. While this is happening, the CV library will be running a live feed from the upward-facing camera and continuously look for and display the
            // fiducial. Once the user confirms they are positioned over the first fiducial, the state machine will transition to DETECT_FIDUCIAL_1.

            // MANUAL CONTROL FROM UI TO JOG GANTRY HERE.

            // IF (USER_CONFIRMS_POSITION) THEN
            //     Transition to DETECT_FIDUCIAL_1.
            // ELSE
            //     CV.REQUEST_FRAME();
            //     CV.DETECT_FIDUCIAL();
            //     CV.UPDATE_DISPLAY();
            // ENDIF
            break;

        case FlowControlState::DETECT_FIDUCIAL_1:
            // Trigger the gantry camera to capture an image, run the CV detection for the first
            // fiducial, and store the detected X/Y/Z pixel coordinates for this fiducial.

            // CV.REQUEST_FRAME();
            // CV.DETECT_FIDUCIAL_1();

            // GRBL.GET_CURRENT_POSITION(); // Get the current XYZ position of the gantry from GRBL.
            // CALCULATE CAMERA_TO_GANTRY_OFFSET(); // Use the known physical offset of the camera from the gantry center to calculate the XYZ coordinates of the camera
            // in the gantry reference frame. CV.PIXEL_TO_3D(); CV.STORE_FIDUCIAL_1_COORDS(); Transition to MOVE_TO_FIDUCIAL_2.
            break;

        case FlowControlState::MOVE_TO_FIDUCIAL_2:
            // MANUAL CONTROL FROM UI TO JOG GANTRY HERE.

            // IF (USER_CONFIRMS_POSITION) THEN
            //     Transition to DETECT_FIDUCIAL_2.
            // ELSE
            //     CV.REQUEST_FRAME();
            //     CV.DETECT_FIDUCIAL();
            //     CV.UPDATE_DISPLAY();
            // ENDIF
            break;

        case FlowControlState::DETECT_FIDUCIAL_2:
            // Trigger the gantry camera to capture an image, run the CV detection for the second
            // fiducial, and store the detected X/Y/Z pixel coordinates for this fiducial.

            // CV.REQUEST_FRAME();
            // CV.DETECT_FIDUCIAL_2();

            // GRBL.GET_CURRENT_POSITION(); // Get the current XYZ position of the gantry from GRBL.
            // CALCULATE CAMERA_TO_GANTRY_OFFSET(); // Use the known physical offset of the camera from the gantry center to calculate the XYZ coordinates of the camera
            // in the gantry reference frame. CV.PIXEL_TO_3D(); CV.STORE_FIDUCIAL_2_COORDS(); Transition to MOVE_TO_FIDUCIAL_3.
            break;

        case FlowControlState::MOVE_TO_FIDUCIAL_3:
            // MANUAL CONTROL FROM UI TO JOG GANTRY HERE.

            // IF (USER_CONFIRMS_POSITION) THEN
            //     Transition to DETECT_FIDUCIAL_3.
            // ELSE
            //     CV.REQUEST_FRAME();
            //     CV.DETECT_FIDUCIAL();
            //     CV.UPDATE_DISPLAY();
            // ENDIF
            break;

        case FlowControlState::DETECT_FIDUCIAL_3:
            // Trigger the gantry camera to capture an image, run the CV detection for the third
            // fiducial, and store the detected X/Y/Z pixel coordinates for this fiducial.

            // CV.REQUEST_FRAME();
            // CV.DETECT_FIDUCIAL_3();

            // GRBL.GET_CURRENT_POSITION(); // Get the current XYZ position of the gantry from GRBL.
            // CALCULATE CAMERA_TO_GANTRY_OFFSET(); // Use the known physical offset of the camera from the gantry center to calculate the XYZ coordinates of the camera
            // in the gantry reference frame. CV.PIXEL_TO_3D(); CV.STORE_FIDUCIAL_3_COORDS(); Transition to MOVE_TO_FIDUCIAL_4.
            break;

        case FlowControlState::MOVE_TO_FIDUCIAL_4:
            // MANUAL CONTROL FROM UI TO JOG GANTRY HERE.

            // IF (USER_CONFIRMS_POSITION) THEN
            //     Transition to DETECT_FIDUCIAL_4.
            // ELSE
            //     CV.REQUEST_FRAME();
            //     CV.DETECT_FIDUCIAL();
            //     CV.UPDATE_DISPLAY();
            // ENDIF
            break;

        case FlowControlState::DETECT_FIDUCIAL_4:
            // Trigger the gantry camera to capture an image, run the CV detection for the fourth
            // fiducial, and store the detected X/Y/Z pixel coordinates for this fiducial.

            // CV.REQUEST_FRAME();
            // CV.DETECT_FIDUCIAL_4();

            // GRBL.GET_CURRENT_POSITION(); // Get the current XYZ position of the gantry from GRBL.
            // CALCULATE CAMERA_TO_GANTRY_OFFSET(); // Use the known physical offset of the camera from the gantry center to calculate the XYZ coordinates of the camera
            // in the gantry reference frame. CV.PIXEL_TO_3D(); CV.STORE_FIDUCIAL_4_COORDS(); Transition to CALCULATE_BOARD_TRANSFORM.
            break;

        case FlowControlState::CALCULATE_BOARD_TRANSFORM:
            // Use the stored 3D coordinates of the and give them to the Board class to calculate the PCB's position and angle relative to the gantry.

            // BOARD.SET_BOARD_OFFSET();
            // Transition to FEEDER_SAFE_START_STATE to begin the pick and place job sequence.
            break;

        // ==========================================
        // PICK AND PLACE JOB SEQUENCE
        // ==========================================
        case FlowControlState::FEEDER_SAFE_START_STATE:
            // This is just a start state marker for the pick and place job sequence. It allows the UI to jump directly to this point for testing without
            // running the homing and vision calibration sequences first. It also serves as a point for the state machine to resume from if the machine is paused or
            // interrupted during the pick and place job. The state machine can return to this known safe state and re-run the pick and place sequence without needing to
            // redo the homing and vision calibration sequences.

            // Do any cleanup here.

            // Transition to GET_NEXT_COMPONENT.
            break;

        case FlowControlState::GET_NEXT_COMPONENT:
            // Get the next component from the job queue. If there are no more components, transition to JOB_COMPLETE.

            // CURRENT_COMPONENT = JOB.GET_NEXT_COMPONENT();
            // IF (CURRENT_COMPONENT == NULL) THEN
            //     Transition to JOB_COMPLETE.
            // ELSE
            //     // Check if the feeder has the next component ready. If not, transition to WAIT_FOR_USER_CUTTAPE_RELOAD to wait for the operator to reload the cut tape
            //     and confirm via the UI that the next component is ready. IF (!FEEDER.NEXT_COMPONENT_READY()) THEN
            //          Transition to WAIT_FOR_USER_CUTTAPE_RELOAD.
            //     ELSE
            //          Transition to ADVANCE_FEEDER.
            //     ENDIF
            // ENDIF
            break;

        case FlowControlState::WAIT_FOR_USER_CUTTAPE_RELOAD:
            // Wait for the operator to reload the cut tape and confirm via the UI that the next component is ready.

            // MANUAL CONTROL FROM UI TO RELOAD CUT TAPE HERE.

            // IF (USER_CONFIRMS_FEEDER_READY) THEN
            //     Transition to PNP_HOME.
            //     // We are skipping the ADVANCE_FEEDER state here because the operator has manually reloaded the cut tape and advanced it to the next component, so we
            //     can just go straight to the homing step for the pick and place sequence.
            // ENDIF
            break;

        case FlowControlState::ADVANCE_FEEDER:
            // Send the command to the feeder to advance the cut tape to the next component.

            // FEEDER.INCREMENT();

            // Transition to PNP_HOME.
            break;

        case FlowControlState::PNP_HOME:
            // Execute the homing sequence to find the machine's XYZ zero reference.
            // GANTRY.HOME();
            // HEAD.HOME();
            // Transition to PNP_HOMING.
            break;

        case FlowControlState::PNP_HOMING:
            // IF (!GRBL.isBusy()) THEN
            //     Transition to PICKUP_SAFE_START_STATE.
            // ENDIF
            break;

        case FlowControlState::PICKUP_SAFE_START_STATE:
            // This is a start state marker for the pickup portion of the pick and place sequence. It allows the state machine to return to a known safe state if the
            // machine is paused or interrupted during the pickup steps. The state machine can return to this state and re-run the pickup steps without needing to redo
            // the homing step.

            // Do any cleanup here.

            // Transition to MOVE_Z_SAFE_PICK.
            break;

        case FlowControlState::MOVE_Z_SAFE_PICK:
            // Move the Z-axis to a safe height for XY travel during the pickup steps.
            // IF (!GRBL.isBusy()) THEN
            //     HEAD.SETHEIGHT(SAFE_Z_HEIGHT);
            //     Transition to MOVE_XY_TO_FEEDER.
            // ENDIF
            break;

        case FlowControlState::MOVE_XY_TO_FEEDER:
            // Move the gantry to position the head over the feeder location for the current component.
            // IF (!GRBL.isBusy()) THEN
            //     GRBL.MOVE_XY(FEEDER.GET_CURRENT_COMPONENT_POSITION());
            //     Transition to LOWER_Z_TO_PICK.
            // ENDIF
            break;

        case FlowControlState::LOWER_Z_TO_PICK:
            // Lower the Z-axis to the pickup height for the current component.
            // IF (!GRBL.isBusy()) THEN
            //     HEAD.SETHEIGHT(CURRENT_COMPONENT.PICKUP_Z_HEIGHT);
            //     Transition to ENABLE_VACUUM.
            // ENDIF
            break;

        case FlowControlState::ENABLE_VACUUM:
            // Turn on the vacuum pump to pick up the component.
            // VACUUM.ON();
            // Transition to DWELL_FOR_VACUUM.
            break;

        case FlowControlState::DWELL_FOR_VACUUM:
            // Wait for a short dwell time to allow the vacuum to securely pick up the component.
            // IF (TIMER_ELAPSED > DWELL_TIME) THEN
            //     Transition to RAISE_Z_FROM_PICK.
            // ENDIF
            break;

        case FlowControlState::RAISE_Z_FROM_PICK:
            // Raise the Z-axis to the safe travel height with the component in tow.
            // IF (!GRBL.isBusy()) THEN
            //     HEAD.SETHEIGHT(SAFE_Z_HEIGHT);
            //     Transition to PLACE_DETECT_SAFE_START_STATE.
            // ENDIF
            break;

        case FlowControlState::PLACE_DETECT_SAFE_START_STATE:
            // This is a start state marker for the placement and vision detection portion of the pick and place sequence. It allows the state machine to return to a
            // known safe state if the machine is paused or interrupted during the placement and vision detection steps. The state machine can return to this state and
            // re-run the placement and vision detection steps without needing to redo the homing and pickup steps.

            // Do any cleanup here.

            // Transition to MOVE_XY_TO_UPWARD_CAMERA.
            break;

        case FlowControlState::MOVE_XY_TO_UPWARD_CAMERA:
            // Move the gantry to position the component under the upward-facing camera for vision-based placement
            // IF (!GRBL.isBusy()) THEN
            //     GRBL.MOVE_XY(UPWARD_CAMERA_POSITION);
            //     Transition to DETECT_COMPONENT_POSE.
            // ENDIF
            break;

        case FlowControlState::DETECT_COMPONENT_POSE:
            // Trigger the upward-facing camera to capture an image, run the CV detection for the component
            // to determine the component's precise XY offset and angle relative to the target placement location.

            // CV.REQUEST_FRAME();
            // CV.DETECT_COMPONENT_POSE();
            // CALCULATE_CORRECTED_PLACEMENT_POSITION();
            // GRBL.MOVE_XY(CORRECTED_PLACEMENT_POSITION);
            // Transition to MOVE_TO_CORRECTED_POSITION.
            break;

        case FlowControlState::MOVING_TO_CORRECTED_POSITION:
            // Move the gantry to position the component over the corrected placement location based on the vision detection results.
            // IF (!GRBL.isBusy()) THEN
            //     Transition to ROTATE_HEAD_A_AXIS.
            // ENDIF
            break;

        case FlowControlState::ROTATE_HEAD_A_AXIS:
            // Rotate the head's A-axis to the correct angle for placement based on the vision detection results.
            // IF (!GRBL.isBusy()) THEN
            //     HEAD.INCREMENT(CORRECTED_PLACEMENT_ANGLE);
            //     Transition to LOWER_Z_TO_DROP.
            // ENDIF
            break;

        case FlowControlState::LOWER_Z_TO_DROP:
            // Lower the Z-axis to the drop height for the current component.
            // IF (!GRBL.isBusy()) THEN
            //     HEAD.SETHEIGHT(CURRENT_COMPONENT.PLACEMENT_Z_HEIGHT);
            //     Transition to DISABLE_VACUUM.
            // ENDIF
            break;

        case FlowControlState::DISABLE_VACUUM:
            // Turn off the vacuum pump to release the component onto the PCB.
            // VACUUM.OFF();
            // Transition to DWELL_FOR_RELEASE.
            break;

        case FlowControlState::DWELL_FOR_RELEASE:
            // Wait for a short dwell time to allow the component to settle onto the PCB after being released.
            // IF (TIMER_ELAPSED > DWELL_TIME) THEN
            //     Transition to RAISE_Z_FROM_DROP.
            // ENDIF
            break;

        case FlowControlState::RAISE_Z_FROM_DROP:
            // Raise the Z-axis back to the safe travel height after placing the component.
            // IF (!GRBL.isBusy()) THEN
            //     HEAD.SETHEIGHT(SAFE_Z_HEIGHT);
            //     Transition to GET_NEXT_COMPONENT.
            // ENDIF
            break;

        // ==========================================
        // ERROR HANDLING & INTERRUPTS
        // ==========================================
        case FlowControlState::PAUSE:
            // The machine is paused. Wait here until the operator resumes the machine from the UI.
            // IF (USER_RESUMES_MACHINE) THEN
            //     Transition to AWAIT_OPERATOR_RESUME.
            // ENDIF
            break;

        case FlowControlState::AWAIT_OPERATOR_RESUME:
            // The machine is waiting for the operator to resume the machine from the UI after being paused or after an error has occurred and been addressed by the
            // operator. Transition back to the appropriate state based on where the machine was paused or where the error occurred. IF (RESUME_FROM_VISION_CALIBRATION)
            // THEN
            //     Transition to BOARD_DETECT_SAFE_START_STATE.
            // ELSE IF (RESUME_FROM_PICK_AND_PLACE) THEN
            //     Transition to FEEDER_SAFE_START_STATE.
            // ELSE IF (RESUME_FROM_ERROR) THEN
            //     Transition to MACHINE_IS_STUPID.
            // ENDIF
            break;

        case FlowControlState::MACHINE_IS_STUPID:
            // This is a catch-all state for when the machine is in an undefined or unexpected state, likely due to an error or bug. The machine will stay in this state
            // until the operator intervenes to reset the machine and return it to a known safe state. This state can be used to trigger error indicators (e.g. flashing
            // lights, error messages on the UI) to alert the operator that the machine is in an error state and needs attention.
            break;
    }
}
