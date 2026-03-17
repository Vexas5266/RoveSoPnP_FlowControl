
#include "flowControl.hpp"
#include "math.h"
#include <thread>

#define ARM_FlowControl_FILE "../board/ArmBoard_Hardware-all-pos.csv"
#define CORE_FlowControl_FILE "../board/CoreBoard-all-pos.csv"

FlowControl::FlowControl(const char* commPort) {

    cout << "Init FlowControl..." << endl;
    grbl = std::make_shared<GRBL>();

    #if (INIT_COMM)
        //Start comm, fill csv
        cout << "Init Comm..." << endl;
        if (grbl.comm.setupComm(commPort) == false) {
            cout << "COM SETUP FAILED" << endl;
            return;
        }
    #else
        return;
    #endif

    //Flush startup
    this_thread::sleep_for(chrono::milliseconds(5000));
    grbl->comm.readLine(); //Flush return
    cout << "GRBL Startup:  ";
    grbl->comm.readLine(); //Startup

    //Init GRBL
    cout << "GRBL Initializing..." << endl;
    grbl->comm.writeLine("?");
    cout << "Startup Status: ";
    grbl->comm.readLine(); //Status
    grbl->comm.readLine(); //Flush ok

    //Send GRBL setup commands
    cout << "Sending GRBL setup commands..." << endl;
    //TODO: Add setup commands (homing, feed, units, etc.)

    cout << "FlowControl Init Complete." << endl;

    led1 = new LED(grbl);
    led2 = new LED(grbl);

    return;
}

void FlowControl::tickStateMachine()
{
    switch (m_current_state)
    {
        // Main state machine
    }
}

FlowControlState FlowControl::advanceComponent()
{
    FlowControlState next_state = FlowControlState::IDLE;
    components_status_t status = m_components->incrementCurrentComponent();

    if (status == SAME_CUTTAPE) {
        //Feed next component
        //Tell Feeder to step forward
        next_state = FlowControlState::PICK;
    }
    else if (status == CHANGE_CUTTAPE) next_state = FlowControlState::RELOAD;
    else if (status == FINAL_CUTTAPE) next_state = FlowControlState::IDLE;

    return next_state;
}

void FlowControl::setState(FlowControlState state)
{
    m_previous_state = m_current_state;
    m_current_state = state;
}

FlowControlState FlowControl::getState()
{
    return m_current_state;
}

FlowControlState FlowControl::getPreviousState()
{
    return m_previous_state;
}

void FlowControl::updateComponents(const char* posFile)
{
    m_components = make_unique<Components>(posFile);
}