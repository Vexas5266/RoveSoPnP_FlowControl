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

#define SPROCKT_R 10 //mm
#define FEEDER_A 'B'
#define HEAD_A 'A'
#define Z_TRAVEL 10
#define PNP_SPEED 1000

enum class FlowControlState {
    IDLE,
    RELOAD,
    PICK,
    STOP,
    COUNT
};

enum class Places {
    ORIGIN,
    FEEEDER,
    INSPECT,
    CNT
};

const coords_t places[(int)Places::CNT] = {
    { 0, 0,   0,   0 }, /* Origin */
    { 50, 20,   0,   0 },     /* Feeder    */
    {  100,  70,   0,   0 },     /* Inspect   */
};

class FlowControl {
    private:
        unique_ptr<Components> m_components;

        FlowControlState m_current_state = FlowControlState::STOP;
        FlowControlState m_previous_state;
        uint8_t m_time = 0;

    public:

        FlowControl(const char* commPort);

        ~FlowControl() {
            cout << "FlowControl DeInit..." << endl;
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

};

#endif /* FLOWCONTROL_H */