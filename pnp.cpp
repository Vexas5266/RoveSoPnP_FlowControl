
#include "pnp.hpp"

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
        cout << "Cut Tape not found!!  " << component.ref << endl;
        //Ask user to fill in, or if dont want, then return
        cuttape = {-1, -1, (orientation_t)0};
    } else {
        cout << "Found cut tape!!  " << component.ref << endl;
        cuttape = {look_it->second.pitch, look_it->second.width, look_it->second.orient};
    }

    placement_map[{component.value, cuttape}].push_back(component);
}

void PnP::addComponent(component_t component, cuttape_t cuttape)
{
    placement_map[{component.value, cuttape}].push_back(component);
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

    string cmd_g = "G90\rG1 X" + to_string(pos.x) + " Y" + to_string(pos.y) + " Z" + to_string(pos.z);
    grbl.comm.writeLine(cmd_g);

    ok = grbl.waitForCommand();
    if (ok == GRBL_OK) ok = grbl.waitForMotion();

    m_ok = ok;
}

void PnP::setAngle(int degrees, char axis)
{
    if (!m_ok) return;

    bool ok = true;

    string cmd_g = "G90\rG1 " + to_string(axis) + to_string(degrees);
    grbl.comm.writeLine(cmd_g);

    ok = grbl.waitForCommand();
    if (ok == GRBL_OK) ok = grbl.waitForMotion();

    m_ok = ok;
}

void PnP::incrementAngle(int degrees, char axis)
{
    if (!m_ok) return;

    bool ok = true;

    string cmd_g = "G91\rG1 " + to_string(axis) + to_string(degrees);
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
    map<tuple<string, cuttape_t>, vector<component_t>>::iterator u_it = placement_map.begin();
    vector<component_t>::iterator c_it = cuttape_it->second.begin();
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
    cuttape_it = placement_map.begin();
    component_it = cuttape_it->second.begin();
}