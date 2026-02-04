
#include "pnp.hpp"
#include "math.h"
#include <thread>

using namespace std;

int comp_count = 0;

void PnP::tickStateMachine()
{
    switch (m_current_state)
        {
            case PICK: {
                cout << "FC: Pick state:  " << components.getCurrentComponent().ref << endl;

                /*  
                    increment feeder
                    Go to feeder coords
                    Maybe: Increment by CV offsets
                    Vacuum on
                    Lower Z
                    Up Z
                */

                feedComponent();
                setPosition_Global(places[FEEEDER_P]);
                pickComponent();
                
                setState(ORIENT);
                break;
            }
            case ORIENT: {
                cout << "FC: Orient state:  " << components.getCurrentComponent().ref << endl;

                /*
                    Go to inspect coords
                    Orient component
                    Increment by CV rotational offsets
                    Record CV XYZ offsets
                */
                
                setPosition_Global(places[INSPECT_P]);
                orientComponent(); //Cancel out rotation in pocket

                coords_t comp_offset = {0, 0, 0, 0}; //Get from CV
                updateCVOffset(comp_offset);

                setState(PLACE);
                break;
            }
            case PLACE: {
                cout << "FC: Place state:  " << components.getCurrentComponent().ref  << endl;

                /*
                    Go to PCB coords
                    Maybe: Increment by recorded CV XYZ offsets
                    Lower Z
                    Vacuum off
                    Up Z
                */

                setPosition_PCB({components.getCurrentComponent().posX, components.getCurrentComponent().posY, 0, components.getCurrentComponent().rotation});
                placeComponent();


                /*
                    switch(Components::advanceComponent())
                    case 0: state = stop
                    case 1: state = pick
                    case 2: state = reload 
                */

                comp_count++;
                
                state_t next_state = advanceComponent();
                setState(next_state);

                break;
            }
            case PAUSE: {
                cout << "FC: Pause state" << endl;

                //Wait for JSON to update
                setState(getPreviousState());
                break;
            }
            case ERROR: {
                cout << "FC: Error state" << endl;
                
                //Let QT app know
                handleError();
                break;
            }
            case RELOAD: {
                cout << "FC: Reload state:  P: " << components.getCurrentComponent().package << "  V: " << components.getCurrentComponent().value << "  Tape: " << components.getCurrentCutTape().width << endl;

                /*
                    Tell user which new cuttape to load
                    Wait for user to reload and go
                */
                
                setState(PICK);
                break;
            }
            case MANUAL: {
                cout << "FC: Manual state" << endl;

                /* Wait for user to finish */

                setState(getPreviousState());
                break;
            }
            case STOP:
                break;
            
        }

        /* 
            Poll from app interface, set state 
        */

        if( !isOK() ) setState(ERROR);

}

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

void PnP::setPosition_PCB(coords_t pos)
{
    if (!m_ok) return;

    bool ok = true;

    //Change to PCB workspace
    string cmd_g = "G55";
    ok = grbl.sendCommand(cmd_g);

    cmd_g = "G90";
    ok = grbl.sendCommand(cmd_g);
    
    cmd_g = "G1 F"+ to_string(PNP_SPEED) + " X" + to_string(pos.x + m_CV_offset.x) + " Y" + to_string(pos.y + m_CV_offset.y);
    if (ok) ok = grbl.sendMotion(cmd_g);

    if (ok) incrementHead(pos.r + m_CV_offset.r);

    m_ok = ok;
}

void PnP::setPosition_Global(coords_t pos)
{
    if (!m_ok) return;

    bool ok = true;

    //Change to global workspace
    string cmd_g = "G54";
    ok = grbl.sendCommand(cmd_g);

    cmd_g = "G90";
    ok = grbl.sendCommand(cmd_g);
    
    cmd_g = "G1 F"+ to_string(PNP_SPEED) + " X" + to_string(pos.x) + " Y" + to_string(pos.y);
    if (ok) grbl.sendMotion(cmd_g);

    m_ok = ok;
}

void PnP::incrementHead(int degrees)
{
    if (!m_ok) return;

    bool ok = true;

    string cmd_g = "G91";
    ok = grbl.sendCommand(cmd_g);

    cmd_g = "G1 F" + to_string(PNP_SPEED) + " A" + to_string(degrees);
    if (ok) grbl.sendMotion(cmd_g);

    m_ok = ok;
}

void PnP::feedComponent()
{
    if (!m_ok) return;

    bool ok = true;

    string cmd_g = "G91";
    ok = grbl.sendCommand(cmd_g);

    cmd_g = "G1 F" + to_string(PNP_SPEED) + " B" + to_string(components.getCurrentCutTape().pitch);
    if (ok) grbl.sendMotion(cmd_g);

    m_ok = ok;
}

void PnP::orientComponent()
{

    int q; //degrees
    q = -(orientations_a[components.getCurrentCutTape().orient]);

    cout << "   Orient: " << q << endl;

    incrementHead(q);
}

void PnP::pickComponent()
{
    cout << "   Picking..." << endl;
    if (!m_ok) return;

    bool ok = true;

    string cmd_g = "G90";
    ok = grbl.sendCommand(cmd_g);
    
    cmd_g = "G1 F"+ to_string(PNP_SPEED) + " Z0";
    if (ok) grbl.sendMotion(cmd_g);

    //Vacuum on

    this_thread::sleep_for(chrono::milliseconds(500));

    cmd_g = "G1 F"+ to_string(PNP_SPEED) + " Z" + to_string(Z_TRAVEL);
    if (ok) grbl.sendMotion(cmd_g);

    m_ok = ok;
}

void PnP::placeComponent()
{
    cout << "   Placing..." << endl;
    if (!m_ok) return;

    bool ok = true;

    string cmd_g = "G90";
    ok = grbl.sendCommand(cmd_g);
    
    cmd_g = "G1 F"+ to_string(PNP_SPEED) + " Z0";
    if (ok) grbl.sendMotion(cmd_g);

    //Vacuum off

    this_thread::sleep_for(chrono::milliseconds(500));

    cmd_g = "G1 F"+ to_string(PNP_SPEED) + " Z" + to_string(Z_TRAVEL);
    if (ok) grbl.sendMotion(cmd_g);

    m_ok = ok;
}

status_t PnP::updateComponents(const char* posFile)
{
    if (m_current_state != STOP) return (status_t)0; //TODO: update with new errors

    components.parseCSV(posFile);
    components.fillLostCuttapes();
    components.printComponents();

    return (status_t)1; //TODO: update with new errors
}

void PnP::updateCVOffset(coords_t offset)
{
    m_CV_offset = offset;
}

void PnP::readFiducials()
{
    /*
        Manually jog to feducials
        Record offsets {x, y, z?, r} (not for m_CV_offset)
        Set GRBL workspace for PCB
    */

    coords_t PCB_offset = {0, 160, 0, 0}; //Get from CV

    string cmd_g = "G10 L2 P2 X" + to_string(PCB_offset.x) + " Y" + to_string(PCB_offset.y) + " A" + to_string(PCB_offset.r);
    grbl.sendCommand(cmd_g);

}