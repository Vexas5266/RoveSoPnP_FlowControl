#ifndef PNP_H
#define PNP_H

#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <iostream>
#include "grbl.hpp"

#define GRBL_OK true
#define SPROCKT_R 10 //mm
#define FEEDER_A 'B'
#define HEAD_A 'A'

using namespace std;

enum orientation_t {
    NA_O,
    C1_O,
    C2_O,
    C3_O,
    C4_O,
    M1_O,
    CNT_O
};

extern const int orientations_a[CNT_O];

struct cuttape_t {
    float pitch;
    float width;
    orientation_t orient;
};

struct component_t {
    string ref;
    string value;
    string package;
    float posX;
    float posY;
    float rotation;
    string side;
    cuttape_t tape;
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
        map<tuple<string, string>, vector<component_t>> placement_map;

        map<tuple<string, string>, vector<component_t>>::iterator unique_it = placement_map.begin();
        vector<component_t>::iterator component_it = unique_it->second.begin();

        state_t m_current_state = PICK;
        state_t m_previous_state = STOP;
        bool m_ok = true;

    public:
        void addComponent(component_t component);
        state_t advanceComponent();
        void setState(state_t state);
        state_t getState();
        state_t getPreviousState(); //Need? Resuming cant go to previous state, dont want to redo
        bool isOK();

        void handleError();

        void setPosition(coords_t pos);
        void setAngle(int degrees, char axis);
        void incrementAngle(int degrees, char axis);

        void feedComponent();
        void orientComponent();

        GRBL grbl;

};

#endif /* PNP_H */