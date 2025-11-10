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

#define GRBL_OK true
#define SPROCKT_R 10 //mm
#define FEEDER_A 'B'
#define HEAD_A 'A'

using namespace std;

struct component_t {
    string ref;
    string value;
    string package;
    float posX;
    float posY;
    float rotation;
    string side;
};

struct coords_t {
    float x;
    float y;
    float z;
};

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
        map<tuple<string, cuttape_t>, vector<component_t>> placement_map;

        map<tuple<string, cuttape_t>, vector<component_t>>::iterator cuttape_it;
        vector<component_t>::iterator component_it;

        state_t m_current_state = PICK;
        state_t m_previous_state = STOP;
        bool m_ok = true;

    public:

        void addComponentLookUp(component_t component);
        void addComponent(component_t component, cuttape_t cuttape);
        state_t advanceComponent();
        component_t getCurrentComponent();
        cuttape_t getCurrentCutTape();

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

        void printComponents();
        void initIterators();

        GRBL grbl;

};

#endif /* PNP_H */