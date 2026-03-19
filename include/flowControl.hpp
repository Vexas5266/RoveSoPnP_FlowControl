#ifndef FLOWCONTROL_H
#define FLOWCONTROL_H

#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <iostream>
#include "grbl.hpp"
#include "tapeLookup.hpp"
#include "board.hpp"
#include "LED.hpp"
#include "head.hpp"
#include "gantry.hpp"
#include "feeder.hpp"

enum class FlowControlState {
    IDLE,
    RELOAD,
    PICK,
    STOP,
    COUNT
};

class FlowControl {
    private:
        std::unique_ptr<Components> m_components;

        FlowControlState m_current_state = FlowControlState::STOP;
        FlowControlState m_previous_state;
        uint8_t m_time = 0;

    public:

        FlowControl(const char* commPort);

        ~FlowControl() {
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