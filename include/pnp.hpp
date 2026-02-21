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
#include "components.hpp"

#define GRBL_OK true
#define SPROCKT_R 10 //mm
#define FEEDER_A 'B'
#define HEAD_A 'A'
#define Z_TRAVEL 10
#define PNP_SPEED 1000

using namespace std;

extern int comp_count;

enum state_t {
    STOP,
    IDLE,
    PICK,
    ORIENT,
    PLACE,
    PAUSE,
    ERROR,
    RELOAD,
    MANUAL
};

enum status_t {
    GOOD_E,
    TIMEOUT_E,
    RESPONSE_E
};

enum places_t {
    ORIGIN_P,
    FEEEDER_P,
    INSPECT_P,
    CNT_P
};

struct coords_t {
    float x;
    float y;
    float z;
    float r;
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

        coords_t m_CV_offset = {0, 0, 0, 0};

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

        void updateCVOffset(coords_t offset);
        void updateComponents(const char* posFile);
        void readFiducials();

        GRBL grbl;

};

#endif /* PNP_H */