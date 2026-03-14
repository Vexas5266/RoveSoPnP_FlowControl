#ifndef PNP_H
#define PNP_H

#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <iostream>
#include "grbl.hpp"
#include "tapeLookup.hpp"
#include "board.hpp"

#define GRBL_OK true
#define SPROCKT_R 10 //mm
#define FEEDER_A 'B'
#define HEAD_A 'A'
#define Z_TRAVEL 10
#define PNP_SPEED 1000

using namespace std;

enum state_t {
    STOP,
    IDLE,
    PICK,
    ORIENT,
    PLACE,
    PAUSE,
    ERROR,
    RELOAD,
    SETUP,
    MANUAL
};

enum places_t {
    ORIGIN_P,
    FEEEDER_P,
    INSPECT_P,
    CNT_P
};

const coords_t places[CNT_P] = {
    { 0, 0,   0,   0 }, /* Origin */
    { 50, 20,   0,   0 },     /* Feeder    */
    {  100,  70,   0,   0 },     /* Inspect   */
};

class PnP {
    private:
        unique_ptr<Components> m_components;

        state_t m_current_state = IDLE;
        state_t m_previous_state = STOP;
        uint8_t m_time = 0;

    public:

        PnP(const char* commPort);

        ~PnP() {
            cout << "PnP DeInit..." << endl;
            grbl.comm.closeComm();
        }

        state_t advanceComponent();
        void setState(state_t state);
        state_t getState();
        state_t getPreviousState();

        void tickStateMachine();

        void setPosition_Global(coords_t pos);
        void setPosition_PCB(coords_t pos);
        void incrementHead(int degrees);
        
        void pickComponent();
        void placeComponent();
        void feedComponent();
        void orientComponent();

        void updateComponents(const char* posFile);

        GRBL grbl;

};

#endif /* PNP_H */