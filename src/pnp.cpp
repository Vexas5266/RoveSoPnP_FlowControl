
#include "pnp.hpp"
#include "math.h"
#include <thread>

using namespace std;

#define ARM_PNP_FILE "../board/ArmBoard_Hardware-all-pos.csv"
#define CORE_PNP_FILE "../board/CoreBoard-all-pos.csv"

int comp_count = 0;

PnP::PnP(const char* commPort) {

    cout << "Init PnP..." << endl;

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

    //Flush startup
    this_thread::sleep_for(chrono::milliseconds(5000));
    grbl.comm.readLine(); //Flush return
    cout << "GRBL Startup:  ";
    grbl.comm.readLine(); //Startup

    //Init GRBL
    cout << "GRBL Initializing..." << endl;
    grbl.comm.writeLine("?");
    cout << "Startup Status: ";
    grbl.comm.readLine(); //Status
    grbl.comm.readLine(); //Flush ok

    //Send GRBL setup commands
    cout << "Sending GRBL setup commands..." << endl;
    //TODO: Add setup commands (homing, feed, units, etc.)

    cout << "Getting PCB Offsets..." << endl;

    readFiducials();

    cout << "PnP Init Complete." << endl;

    return;
}

void PnP::tickStateMachine()
{
    switch (m_current_state)
        {
            case IDLE: {
                cout << "FC: Idle state" << endl;
                cout << "Placed: " << comp_count << endl;
                /*
                    Need to get new CSV and parse
                    get new CV offsets
                    Read new fidicials
                    Reset component count
                */
                
                m_time++;
                if (m_time == 1) {
                    updateComponents(ARM_PNP_FILE); 
                    setState(PICK);
                }
                else setState(STOP);
                comp_count = 0;
                break;
            }
            case PICK: {
                cout << "FC: Pick state:  " << m_components->getCurrentComponent().ref << endl;

                /*  
                    increment feeder
                    Go to feeder coords
                    Maybe: Increment by CV offsets
                    Vacuum on
                    Lower Z
                    Up Z
                */

                setPosition_Global(places[FEEEDER_P]);
                pickComponent();
                
                setState(ORIENT);
                break;
            }
            case ORIENT: {
                cout << "FC: Orient state:  " << m_components->getCurrentComponent().ref << endl;

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
                cout << "FC: Place state:  " << m_components->getCurrentComponent().ref  << endl;

                /*
                    Go to PCB coords
                    Maybe: Increment by recorded CV XYZ offsets
                    Lower Z
                    Vacuum off
                    Up Z
                */

                setPosition_PCB({m_components->getCurrentComponent().posX, m_components->getCurrentComponent().posY, 0, m_components->getCurrentComponent().rotation});
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
                break;
            }
            case RELOAD: {
                cout << "FC: Reload state:  P: " << m_components->getCurrentComponent().package << "  V: " << m_components->getCurrentComponent().value << "  Tape: " << m_components->getCurrentCutTape().ID << endl;

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

}

state_t PnP::advanceComponent()
{
    state_t next_state = IDLE;
    components_status_t status = m_components->incrementCurrentComponent();

    if (status == SAME_CUTTAPE) {
        //Feed next component
        feedComponent();
        next_state = PICK;
    }
    else if (status == CHANGE_CUTTAPE) next_state = RELOAD;
    else if (status == FINAL_CUTTAPE) next_state = IDLE;

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

void PnP::setPosition_PCB(coords_t pos)
{
    string cmd_g = "G54 G90 G1 F"+ to_string(PNP_SPEED) + " X" + to_string(pos.x) + " Y" + to_string(pos.y);
    grbl.sendMotion(cmd_g);

    incrementHead(pos.r);
}

void PnP::setPosition_Global(coords_t pos)
{
    string cmd_g = "G53 G90 G1 F"+ to_string(PNP_SPEED) + " X" + to_string(pos.x) + " Y" + to_string(pos.y);
    grbl.sendMotion(cmd_g);
}

void PnP::incrementHead(int degrees)
{
    string cmd_g = "G91 G1 F" + to_string(PNP_SPEED*4) + " A" + to_string(degrees);
    grbl.sendMotion(cmd_g);
}

void PnP::feedComponent()
{
    string cmd_g = "G91 G1 F" + to_string(PNP_SPEED) + " B" + to_string(m_components->getCurrentCutTape().pitch);
    grbl.sendMotion(cmd_g);
}

void PnP::orientComponent()
{

    int q; //degrees
    q = -(orientations_a[m_components->getCurrentCutTape().orient]);

    cout << "   Orient: " << q << endl;

    incrementHead(q);
}

void PnP::pickComponent()
{
    cout << "   Picking..." << endl;
    string cmd_g = "G90 G1 F"+ to_string(PNP_SPEED) + " Z0";
    grbl.sendMotion(cmd_g);

    //Vacuum on

    // this_thread::sleep_for(chrono::milliseconds(500));

    cmd_g = "G90 G1 F"+ to_string(PNP_SPEED) + " Z" + to_string(Z_TRAVEL);
    grbl.sendMotion(cmd_g);
}

void PnP::placeComponent()
{
    cout << "   Placing..." << endl;
    
    string cmd_g = "G90 G1 F"+ to_string(PNP_SPEED) + " Z0";
    grbl.sendMotion(cmd_g);

    //Vacuum off

    // this_thread::sleep_for(chrono::milliseconds(500));

    cmd_g = "G90 G1 F"+ to_string(PNP_SPEED) + " Z" + to_string(Z_TRAVEL);
    grbl.sendMotion(cmd_g);
}

void PnP::updateComponents(const char* posFile)
{
    m_components = make_unique<Components>(posFile);
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

    coords_t PCB_offset = {10, 160, 0}; //Get from CV

    string cmd_g = "G10 L2 P1 X" + to_string(PCB_offset.x) + " Y" + to_string(PCB_offset.y);
    grbl.sendCommand(cmd_g);

}