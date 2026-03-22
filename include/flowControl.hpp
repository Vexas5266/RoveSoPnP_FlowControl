#ifndef FLOWCONTROL_H
#define FLOWCONTROL_H

#include "LED.hpp"
#include "board.hpp"
#include "feeder.hpp"
#include "flowControlStates.hpp"
#include "gantry.hpp"
#include "grbl.hpp"
#include "head.hpp"
#include "tapeLookup.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class FlowControl
{
    private:
        std::unique_ptr<Components> m_components;

        FlowControlState m_current_state = FlowControlState::IDLE;
        FlowControlState m_previous_state;
        uint8_t m_time = 0;

    public:
        FlowControl(const char* commPort);

        ~FlowControl()
        {
            std::cout << "FlowControl DeInit..." << std::endl;
            grbl->comm.closeComm();
        }

        FlowControlState advanceComponent();
        void setState(FlowControlState state);
        FlowControlState getState();
        FlowControlState getPreviousState();

        void tickStateMachine();

        void updateComponents(const char* posFile);

        std::shared_ptr<GRBL> grbl;
        LED* led1;
        LED* led2;

        Head* head;
        Gantry* gantry;
        Feeder* feeder;
};

#endif /* FLOWCONTROL_H */
