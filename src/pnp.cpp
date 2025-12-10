
#include "pnp.hpp"
#include "math.h"

using namespace std; 

state_t PnP::advanceComponent()
{
    state_t next_state = STOP;
    components.incrementCurrentComponent();

    //Handle iterators
    if (components.getComponent_it() == components.getCutTape_it()->second.end()) 
    {
        components.incrementCurrentCutTape();
        //Replace components
        next_state = RELOAD;
    } else next_state = PICK;

    if (components.getCutTape_it() == components.getPlacementMap()->end()) next_state = STOP;

    return next_state;
}

void PnP::setState(state_t state)
{
    m_previous_state = m_current_state;
    m_current_state = state;
}

state_t PnP::getState()
{
    return m_current_state;
}

state_t PnP::getPreviousState()
{
    return m_previous_state;
}

bool PnP::isOK()
{
    return m_ok;
}

void PnP::handleError()
{
    m_ok = true;
    PnP::setState(STOP);
}

void PnP::setPosition(coords_t pos)
{
    if (!m_ok) return;

    bool ok = true;

    string cmd_g = "G90";
    grbl.comm.writeLine(cmd_g);
    cmd_g = "G1 F"+ speed + " X" + to_string(pos.x) + " Y" + to_string(pos.y) + " Z" + to_string(pos.z);
    grbl.comm.writeLine(cmd_g);

    ok = grbl.waitForCommand();
    if (ok == GRBL_OK) ok = grbl.waitForMotion();

    m_ok = ok;
}

void PnP::setAngle(int degrees, char axis)
{
    if (!m_ok) return;

    bool ok = true;

    string cmd_g = "G90";
    grbl.comm.writeLine(cmd_g);
    cmd_g = "G1 F" + speed + " " + to_string(axis) + to_string(degrees);
    grbl.comm.writeLine(cmd_g);

    ok = grbl.waitForCommand();
    if (ok == GRBL_OK) ok = grbl.waitForMotion();

    m_ok = ok;
}

void PnP::incrementAngle(int degrees, char axis)
{
    if (!m_ok) return;

    bool ok = true;

    string cmd_g = "G91";
    grbl.comm.writeLine(cmd_g);
    cmd_g = "G1 F" + speed + " " + to_string(axis) + to_string(degrees);
    grbl.comm.writeLine(cmd_g);

    ok = grbl.waitForCommand();
    if (ok == GRBL_OK) ok = grbl.waitForMotion();

    m_ok = ok;
}

void PnP::feedComponent()
{

    int q; //degrees
    q = components.getCurrentCutTape().pitch * ( 360 / ( 2*M_PI * SPROCKT_R ) );
    PnP::incrementAngle(q, FEEDER_A);

}

void PnP::orientComponent()
{

    int q; //degrees
    q = components.getComponent_it()->rotation - orientations_a[components.getCurrentCutTape().orient];
    q %= 360;

    PnP::setAngle(q, HEAD_A);
}

bool PnP::init(const char* commPort)
{
    //Start comm, fill csv
    cout << "Init PnP..." << endl;
    if (grbl.comm.setupComm(commPort) == false) {
        cout << "COM SETUP FAILED" << endl;
        return false;
    }

    //Init GRBL
    cout << "GRBL Initializing..." << endl;

    //Flush startup
    this_thread::sleep_for(chrono::milliseconds(2000));
    cout << "GRBL Startup:  " << grbl.comm.readLine() << endl;

    //Send GRBL setup commands
    cout << "Sending GRBL setup commands..." << endl;
    //TODO: Add setup commands (homing, feed, units, etc.)

    cout << "PnP Init Complete." << endl;
    return true;

}