
#include "pnp.hpp"

using namespace std; 

void PnP::addComponent(component_t component)
{
    tuple<string, string> unique_comp = {component.value, component.package};
    placement_map[unique_comp].push_back(component);
}

state_t PnP::advanceComponent()
{
    state_t next_state = STOP;
    component_it++;

    //Handle iterators
    if (component_it == unique_it->second.end()) 
    {
        unique_it++;
        component_it = unique_it->second.begin();
        //Replace components
        next_state = RELOAD;
    } else next_state = PICK;

    if (unique_it == placement_map.end()) next_state = STOP;

    return next_state;
}

component_t PnP::getCurrentComponent()
{
    return *component_it;
}

void PnP::fillCutTapes()
{

    map<tuple<string, string>, vector<component_t>>::iterator comp_it = placement_map.begin();

    
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

    string cmd_g = "G90 G1 X" + to_string(pos.x) + " Y" + to_string(pos.y) + " Z" + to_string(pos.z);
    grbl.comm.writeLine(cmd_g);

    ok = grbl.waitForCommand();
    if (ok == GRBL_OK) ok = grbl.waitForMotion();

    m_ok = ok;
}

void PnP::setAngle(int degrees, char axis)
{
    if (!m_ok) return;

    bool ok = true;

    string cmd_g = "G90 G1 " + to_string(axis) + to_string(degrees);
    grbl.comm.writeLine(cmd_g);

    ok = grbl.waitForCommand();
    if (ok == GRBL_OK) ok = grbl.waitForMotion();

    m_ok = ok;
}

void PnP::incrementAngle(int degrees, char axis)
{
    if (!m_ok) return;

    bool ok = true;

    string cmd_g = "G91 G1 " + to_string(axis) + to_string(degrees);
    grbl.comm.writeLine(cmd_g);

    ok = grbl.waitForCommand();
    if (ok == GRBL_OK) ok = grbl.waitForMotion();

    m_ok = ok;
}

void PnP::feedComponent()
{

    int q; //degrees
    q = component_it->tape.pitch * ( 360 / ( 2*M_PI * SPROCKT_R ) );
    PnP::incrementAngle(q, FEEDER_A);

}

void PnP::orientComponent()
{

    int q; //degrees
    q = component_it->rotation - orientations_a[component_it->tape.orient];
    q %= 360;

    PnP::setAngle(q, HEAD_A);
}

void PnP::printComponents()
{
    map<tuple<string, string>, vector<component_t>>::iterator u_it = placement_map.begin();
    vector<component_t>::iterator c_it = unique_it->second.begin();
    while (u_it != placement_map.end())
    {
        cout << c_it->ref << endl;

        c_it++;

        //Handle iterators
        if (c_it == u_it->second.end()) 
        {
            u_it++;
            c_it = u_it->second.begin();
        }
    }
}

void PnP::initIterators()
{
    unique_it = placement_map.begin();
    component_it = unique_it->second.begin();
}