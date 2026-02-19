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

        state_t m_current_state = PICK;
        state_t m_previous_state = STOP;

        coords_t m_CV_offset = {0, 0, 0, 0};

    public:

        PnP(const char* commPort, const char* posFile) {

            cout << "Init PnP..." << endl;

            cout << "Parse CSV..." << endl;
            components.parseCSV(posFile);
            components.fillLostCuttapes();
            components.printComponents();

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
            grbl.comm.readLine(); //Flush return
            cout << "GRBL Startup:  ";
            grbl.comm.readLine(); //Startup

            //Init GRBL
            cout << "GRBL Initializing..." << endl;
            grbl.comm.writeLine("?");
            cout << "Startup Status: ";
            grbl.comm.readLine(); //Status
            grbl.comm.readLine(); //Flush ok

            //Send GRBL setup commands
            cout << "Sending GRBL setup commands..." << endl;
            //TODO: Add setup commands (homing, feed, units, etc.)

            cout << "Getting PCB Offsets..." << endl;

            readFiducials();

            cout << "PnP Init Complete." << endl;

            return;
        }

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
        status_t updateComponents(const char* posFile);
        void readFiducials();

        GRBL grbl;
        Components components;

};

#endif /* PNP_H */