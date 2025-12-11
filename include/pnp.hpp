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

using namespace std;

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
    FEEEDER_P,
    INSPECT_P,
    PCB_P,
    CV_OFFSET_P,
    CNT_P
};

const coords_t places[CNT_P] = {
    { 100, -100,   0 },     /* Feeder    */
    {  50,  -50,   0 },     /* Inspect   */
    {   0,    0,   0 },     /* PCB       */
    {   0,    0,   0 },     /* CV Offset */
};

class PnP {
    private:

        state_t m_current_state = PICK;
        state_t m_previous_state = STOP;
        bool m_ok = true;

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

            //Init GRBL
            cout << "GRBL Initializing..." << endl;

            //Flush startup
            this_thread::sleep_for(chrono::milliseconds(2000));
            cout << "GRBL Startup:  " << grbl.comm.readLine() << endl;

            //Send GRBL setup commands
            cout << "Sending GRBL setup commands..." << endl;
            //TODO: Add setup commands (homing, feed, units, etc.)

            cout << "PnP Init Complete." << endl;

            return;
        }

        state_t advanceComponent();
        void setState(state_t state);
        state_t getState();
        state_t getPreviousState();
        bool isOK();

        void handleError();

        void setPosition(coords_t pos);
        void setAngle(int degrees, char axis);
        void incrementAngle(int degrees, char axis);

        void feedComponent();
        void orientComponent();

        bool init(const char* commPort);

        GRBL grbl;
        Components components;

};

#endif /* PNP_H */