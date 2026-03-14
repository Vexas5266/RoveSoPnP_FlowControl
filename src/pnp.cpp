
#include "pnp.hpp"
#include "math.h"
#include <thread>

using namespace std;

#define ARM_PNP_FILE "../board/ArmBoard_Hardware-all-pos.csv"
#define CORE_PNP_FILE "../board/CoreBoard-all-pos.csv"

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

    cout << "PnP Init Complete." << endl;

    return;
}

void PnP::tickStateMachine()
{
    switch (m_current_state)
    {
        case IDLE: {
            cout << "FC: Idle state" << endl;
            
            m_time++;
            if (m_time == 1) {
                setState(SETUP);
            }
            else {
                cout << "Placed: " << m_components->getPlacedComponents() << endl;
                setState(STOP);
            }

            break;
        }
        case SETUP: {
            cout << "FC: Setup state" << endl;
            /*
                 == Board setup ==
                Need to get new CSV and parse
                Ask user to fill in lost components or skip them
                Ask user to jog to fiducials to get new CV offsets
            */
            updateComponents(ARM_PNP_FILE);
            // Get manual positions of fiducials
            m_components->setManualFidcucials(0, { -4.028943,      9.441441,       0,      0});
            m_components->setManualFidcucials(1, {-79.463095,    92.957823,     0,      0});
            // m_components->setManualFidcucials(2, {-29.173661,   151.329487,     0,      0});

            m_components->calculateBoardOffset();
            cout << "Offset: X:" << m_components->getBoardOffset().x << " Y:" << m_components->getBoardOffset().y << " R:" << RAD2DEG(m_components->getBoardOffset().r) << endl;

            coords_t test = { 1, 1, 0, DEG2RAD(45) };
            m_components->transformToPCBCoords(&test);
            cout << "PCB: X:" << test.x << " Y:" << test.y << " R:" << RAD2DEG(test.r) << endl;

            m_components->transformToGlobalCoords(&test);
            cout << "PCB: X:" << test.x << " Y:" << test.y << " R:" << RAD2DEG(test.r) << endl;

            setState(STOP); //Change to reload
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