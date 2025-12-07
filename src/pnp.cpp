
#include "pnp.hpp"
#include "parse.hpp"
#include "math.h"

using namespace std; 

void PnP::addComponentLookUp(component_t component)
{
    cuttape_t cuttape;

    map<string, cuttape_t>::iterator look_it = cut_tape_map.begin();
    while (look_it != cut_tape_map.end())
    {
        if (component.package.find(look_it->first) != string::npos) break;

        look_it++;
    }

    if (look_it == cut_tape_map.end()) 
    {
        // Not found in lookup
        notInLookup[{component.value, component.package}].push_back(component);
    } else {
        // cout << "Found cut tape!!  " << component.ref << endl;
        cuttape = {look_it->second.pitch, look_it->second.width, look_it->second.orient};
        placement_map[{component.value, cuttape}].push_back(component);
    }

}

void PnP::fillLostCuttapes()
{
    map<tuple<string, string>, vector<component_t>>::iterator lost_cuttape_it = notInLookup.begin();

    while (lost_cuttape_it != notInLookup.end())
    {
        //Ask for cut tape info from user
        cuttape_t user_cuttape = {-1, -2, NA_O};
        vector<component_t>::iterator individual_comp_it = lost_cuttape_it->second.begin();
        while (individual_comp_it != lost_cuttape_it->second.end())
        {
            placement_map[{individual_comp_it->value, user_cuttape}].push_back(*individual_comp_it);
            individual_comp_it++;
        }

        lost_cuttape_it++;
    }
}

state_t PnP::advanceComponent()
{
    state_t next_state = STOP;
    component_it++;

    //Handle iterators
    if (component_it == cuttape_it->second.end()) 
    {
        cuttape_it++;
        component_it = cuttape_it->second.begin();
        //Replace components
        next_state = RELOAD;
    } else next_state = PICK;

    if (cuttape_it == placement_map.end()) next_state = STOP;

    return next_state;
}

component_t PnP::getCurrentComponent()
{
    return *component_it;
}

cuttape_t PnP::getCurrentCutTape()
{
    return get<1>(cuttape_it->first);
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
    q = getCurrentCutTape().pitch * ( 360 / ( 2*M_PI * SPROCKT_R ) );
    PnP::incrementAngle(q, FEEDER_A);

}

void PnP::orientComponent()
{

    int q; //degrees
    q = component_it->rotation - orientations_a[getCurrentCutTape().orient];
    q %= 360;

    PnP::setAngle(q, HEAD_A);
}

void PnP::printComponents()
{
    int count = 0;
    map<tuple<string, cuttape_t>, vector<component_t>>::iterator u_it = placement_map.begin();
    vector<component_t>::iterator c_it = u_it->second.begin();
    while (u_it != placement_map.end())
    {
        cout << "Ref:" << c_it->ref << " Val:" << c_it->value << " Width:" << get<1>(u_it->first).width << " Pkg:" << c_it->package << endl;

        c_it++;
        count++;
        
        //Handle iterators
        if (c_it == u_it->second.end()) 
        {
            u_it++;
            c_it = u_it->second.begin();
        }
    }

    cout << "Size: " << count << endl;
}

void PnP::initIterators()
{
    cuttape_it = placement_map.begin();
    component_it = cuttape_it->second.begin();
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

void PnP::parseCSV(const char* csvFile)
{
    ifstream file(csvFile);
    string line;
    getline(file, line, '\n');
    while (getline(file, line, '\n')) {
        stringstream ss(line);
        component_t component = {
            parseItemString(ss), //ref
            parseItemString(ss), //value
            parseItemString(ss), //package
            parseItemFloat(ss), //posX
            parseItemFloat(ss), //posY
            parseItemFloat(ss), //rotation
            parseItemString(ss), //side
        };

        PnP::addComponentLookUp(component);
    }
    PnP::initIterators();
}