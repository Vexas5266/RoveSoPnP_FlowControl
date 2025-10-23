#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <math.h>

#include <thread>
#include <chrono>

#include "comm.hpp"

using namespace std;

#define TIMEOUT_E_S 15 //seconds
#define GRBL_OK true
#define SPROCKT_R 10 //mm
#define FEEDER_A 'B'
#define HEAD_A 'A'

enum orientation_t {
    NA_O,
    C1_O,
    C2_O,
    C3_O,
    C4_O,
    M1_O,
    CNT_O
};
typedef struct {
    int pitch;
    int width;
    orientation_t orient;
} cuttape_t;
typedef struct {
    string ref;
    string value;
    string package;
    float posX;
    float posY;
    float rotation;
    string side;
    cuttape_t tape;
} component_t;

typedef struct {
    float x;
    float y;
    float z;
} coords_t;

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

int orientations_a[CNT_O] = {
    0,   /* NA */
    90,  /* C1 */
    0,   /* C2 */
    -90, /* C3 */
    180, /* C4 */
    0,   /* M1 */
};

coords_t places[CNT_P] = {
    { 100, -100,   0 },     /* Feeder    */
    {  50,  -50,   0 },     /* Inspect   */
    {   0,    0,   0 },     /* PCB       */
    {   0,    0,   0 },     /* CV Offset */
};

string FC_parseItemString(stringstream& s);
float FC_parseItemFloat(stringstream &s);
int FC_parseItemInt(stringstream &s);

void FC_msSleep(long int dur); //in seconds
bool GRBL_waitForMotion(int fd);
bool GRBL_waitForCommand(int fd);

bool PnP_setPosition(int fd, coords_t pos);
bool PnP_setAngle(int fd, int degrees, char axis);
bool PnP_nextComponent(int fd, component_t comp);
bool PnP_orientComponent(int fd, component_t comp);

map<tuple<string, string>, vector<component_t>> placement_map;
ifstream file("ArmBoard_Hardware-all-pos.csv");

// Need to set initial offsets from testing for Z and A axes
const string speed = "300";
const string init_g = "G20 G94 F" + speed;
const string home_g = "G90 G1 Z0.5\nG28 X Y\nG92 X0 Y0";

state_t PnP_state;
bool PnP_ok = true;

int main() {

    int fd = setupComm();

    string line;
    getline(file, line, '\n');
    while (getline(file, line, '\n')) {
        stringstream ss(line);
        component_t component = {
            FC_parseItemString(ss), //ref
            FC_parseItemString(ss), //value
            FC_parseItemString(ss), //package
            FC_parseItemFloat(ss), //posX
            FC_parseItemFloat(ss), //posY
            FC_parseItemFloat(ss), //rotation
            FC_parseItemString(ss), //side
            // {
            //     FC_parseItemInt(ss), //pitch
            //     FC_parseItemInt(ss), //width
            //     (orientation_t)FC_parseItemInt(ss), //orientation
            // },
        };

        tuple<string, string> unique_comp {component.value, component.package};
        placement_map[unique_comp].push_back(component);
    }

    //Init GRBL
    writeLine(fd, init_g);
    writeLine(fd, home_g);

    //Flush startup
    FC_msSleep(2000);
    cout << "GRBL Startup:  " << readLine(fd) << endl;

    map<tuple<string, string>, vector<component_t>>::iterator unq_it = placement_map.begin();
    vector<component_t>::iterator c_it = unq_it->second.begin();

    PnP_state = PICK;

    while (PnP_state != STOP)
    {
        /* == Hiearchy of States == */
        /* Poll from app interface, set state */

        if (!PnP_ok) PnP_state = ERROR;

        switch (PnP_state)
        {
            case PICK: {
                cout << "FC: Pick state" << endl;

                /*  
                    Set head to 0
                    Go to feeder coords
                    Increment by CV offsets
                    Lower Z
                    Vacuum on
                    Up Z
                */
                
                PnP_state = ORIENT;
                break;
            }
            case ORIENT: {
                cout << "FC: Orient state" << endl;

                /*
                    Go to inspect coords
                    Orient component
                    Increment by CV offsets
                */
                PnP_ok = PnP_setAngle(fd, 90, HEAD_A);

                PnP_state = PLACE;
                break;
            }
            case PLACE: {
                cout << "FC: Place state" << endl;

                /*
                    Go to PCB coords
                    Increment by CV offsets
                    Lower Z
                    Vacuum off
                    Up Z
                    Increment feeder
                */

                c_it++;

                //Handle iterators
                if (c_it == unq_it->second.end()) 
                {
                    unq_it++;
                    c_it = unq_it->second.begin();
                    //Replace components
                    PnP_state = RELOAD;
                } else {
                    PnP_state = PICK;
                }

                if (unq_it == placement_map.end()) PnP_state = STOP;

                break;
            }
            case PAUSE: {
                cout << "FC: Pause state" << endl;

                //Wait for JSON to update -> PnP_state = RUN

                break;
            }
            case ERROR: {
                cout << "FC: Error state" << endl;

                PnP_ok = true;
                PnP_state = STOP;
                break;
            }
            case RELOAD: {
                cout << "FC: Reload state" << endl;

                break;
            }
            case MANUAL: {
                cout << "FC: Manual state" << endl;

                break;
            }
            case STOP:
                break;
            
        }

    }

    cout << "FC: Stop state" << endl;


    close(fd);

    return 0;
} 

string FC_parseItemString(stringstream &s)
{
    string data;
    getline(s, data, ',');
    data = data.substr(1, data.length() - 2);
    // cout << data << endl;
    return data;
}

float FC_parseItemFloat(stringstream &s)
{
    string data;
    getline(s, data, ',');
    // cout << data << endl;
    return stof(data);
}

int FC_parseItemInt(stringstream &s)
{
    string data;
    getline(s, data, ',');
    // cout << data << endl;
    return stoi(data);
}

bool GRBL_isIdle(int fd) 
{
    // Ask GRBL for a status report
    write(fd, "?\n", 2);
    this_thread::sleep_for(chrono::milliseconds(100));
    string resp = readLine(fd);
    return resp.find("Idle") != string::npos;
}

void FC_msSleep(long int dur)
{
    this_thread::sleep_for(chrono::milliseconds(dur));
}

bool GRBL_waitForMotion(int fd)
{
    bool ok = true;
    //Wait for GRBL to be idle
    int timeout = TIMEOUT_E_S;
    while ((!GRBL_isIdle(fd)) && (timeout > 0)) {
        cout << "Running..." << endl;
        timeout--;
        FC_msSleep(1000);
    }
    if (timeout <= 0) {
        cout << "ERROR: MOTION TIMED OUT" << endl;
        ok = false;
    }

    cout << "Motion complete." << endl;
    return ok;
}

bool GRBL_waitForCommand(int fd)
{
    //Wait for ok response
    bool ok = true;

    int timeout = TIMEOUT_E_S;
    while (timeout > 0)
    {
        string response = readLine(fd);
        if (response == "ok") break;
        if (!response.empty()) {
            cout << "GRBL: " << response << endl;
            ok = false;
            break;
        }
        FC_msSleep(1000);
        timeout--;
    }
    if (timeout <= 0) {
        cout << "ERROR: CMND SEND TIMED OUT" << endl;
        ok = false;
    }

    return ok;
}

bool PnP_setPosition(int fd, coords_t pos)
{
    if (!PnP_ok) return false;

    bool ok = true;

    string cmd_g = "G90 G1 X" + to_string(pos.x) + " Y" + to_string(pos.y) + " Z" + to_string(pos.z);
    writeLine(fd, cmd_g);

    ok = GRBL_waitForCommand(fd);
    if (ok == GRBL_OK) ok = GRBL_waitForMotion(fd);

    return ok;
}

bool PnP_setAngle(int fd, int degrees, char axis)
{
    if (!PnP_ok) return false;

    bool ok = true;

    string cmd_g = "G90 G1 " + to_string(axis) + to_string(degrees);
    writeLine(fd, cmd_g);

    ok = GRBL_waitForCommand(fd);
    if (ok == GRBL_OK) ok = GRBL_waitForMotion(fd);

    return ok;
}

bool PnP_nextComponent(int fd, component_t comp)
{
    if (!PnP_ok) return false;

    int q; //degrees
    q = comp.tape.pitch * ( 360 / ( 2*M_PI * SPROCKT_R ) );
    return PnP_setAngle(fd, q, FEEDER_A);

}

bool PnP_orientComponent(int fd, component_t comp)
{
    if (!PnP_ok) return false;

    int q; //degrees
    q = comp.rotation - orientations_a[comp.tape.orient];
    q %= 360;

    return PnP_setAngle(fd, q, HEAD_A);
}