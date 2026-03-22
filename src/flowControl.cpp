
#include "flowControl.hpp"
#include "math.h"
#include <thread>

#define ARM_FlowControl_FILE  "../board/ArmBoard_Hardware-all-pos.csv"
#define CORE_FlowControl_FILE "../board/CoreBoard-all-pos.csv"

FlowControl::FlowControl(const char* commPort)
{
    std::cout << "Init FlowControl..." << std::endl;
    grbl   = std::make_shared<GRBL>(commPort);

    led1   = new LED(grbl);
    led2   = new LED(grbl);

    head   = new Head(grbl);
    gantry = new Gantry(grbl);
    feeder = new Feeder(grbl);

    // Send GRBL setup commands
    std::cout << "Sending GRBL setup commands..." << std::endl;
    // TODO: Add setup commands (homing, feed, units, etc.)

    std::cout << "FlowControl Init Complete." << std::endl;

    return;
}

FlowControlState FlowControl::advanceComponent()
{
    FlowControlState next_state = FlowControlState::IDLE;
    components_status_t status  = m_components->incrementCurrentComponent();

    if (status == SAME_CUTTAPE)
    {
        // Feed next component
        // Tell Feeder to step forward
        next_state = FlowControlState::PICKUP_SAFE_START_STATE;
    }
    else if (status == CHANGE_CUTTAPE)
        next_state = FlowControlState::FEEDER_SAFE_START_STATE;
    else if (status == FINAL_CUTTAPE)
        next_state = FlowControlState::IDLE;

    return next_state;
}

void FlowControl::setState(FlowControlState state)
{
    m_previous_state = m_current_state;
    m_current_state  = state;
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
    m_components = std::make_unique<Components>(posFile);
}
