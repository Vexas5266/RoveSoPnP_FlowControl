
#include "pnp.hpp"
#include "math.h"

using namespace std;

int comp_count = 0;

void PnP::tickStateMachine()
{
    switch (getState())
        {
            case PICK: {
                cout << "FC: Pick state:  " << components.getCurrentComponent().ref << endl;

                /*  
                    Set head to 0
                    increment feeder
                    Go to feeder coords
                    Maybe: Increment by CV offsets
                    Vacuum on
                    Lower Z
                    Up Z
                */

                
                setState(ORIENT);
                break;
            }
            case ORIENT: {
                cout << "FC: Orient state" << endl;

                /*
                    Go to inspect coords
                    Orient component
                    Increment by CV rotational offsets
                    Record CV XYZ offsets
                */
                // rovePnP.setAngle(90, HEAD_A);

                setState(PLACE);
                break;
            }
            case PLACE: {
                cout << "FC: Place state" << endl;

                /*
                    Go to PCB coords
                    Maybe: Increment by recorded CV XYZ offsets
                    Lower Z
                    Vacuum off
                    Up Z
                */

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

void PnP::setPosition(coords_t pos)
{
    if (!m_ok) return;

    bool ok = true;

    string cmd_g = "G90";
    grbl.comm.writeLine(cmd_g);
    cmd_g = "G1 F"+ speed + " X" + to_string(pos.x + m_CV_offset.x) + " Y" + to_string(pos.y + m_CV_offset.y) + " Z" + to_string(pos.z + m_CV_offset.z);
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
    q += m_CV_offset.r;
    q %= 360;

    PnP::setAngle(q, HEAD_A);
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
