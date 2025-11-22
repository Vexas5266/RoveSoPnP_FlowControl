#include <math.h>

#include <thread>
#include <chrono>

#include "pnp.hpp"

using namespace std;

#define PNP_FILE "ArmBoard_Hardware-all-pos.csv"
#define COM_PORT "/dev/tty.usbserial-140"

PnP rovePnP;
int comp_count = 0;

void FC_msSleep(long int dur); //in seconds

// Need to set initial offsets from testing for Z and A axes

int main() {

    /*
    
        Ask User:
        CSV File
        Com port

    */

    if (rovePnP.init(COM_PORT) == false) return 0;

    /*
        Parse CSV and fill in components, add to placement map
        Iterate through each tuple in placement_map
        Compare package to look up packages
            set cuttape config
        else 
            ask user to input or remove from placement map
    */
    rovePnP.parseCSV(PNP_FILE);
    rovePnP.fillLostCuttapes();
    rovePnP.printComponents();

    /*
        Tell user to go to first feducial
        Manually jog to feducial
        CV reads offset
        Increment axes by that offset
        Set XYZ to the feducial coord

        Tell user to jog to second feducial
        CV reads offset
        Increment axes by that offset
        See how this feducial is rotated relative to first
        Calculate board rotation

        Tell user to jog to third feducial
        CV reads offset
        Increment axes by that offset
        See how this feducial is rotated relative to first (on Z)
        Calculate board Z axis rotation
    */

    while (rovePnP.getState() != STOP)
    {

        switch (rovePnP.getState())
        {
            case PICK: {
                cout << "FC: Pick state:  " << rovePnP.getCurrentComponent().ref << endl;

                /*  
                    Set head to 0
                    increment feeder
                    Go to feeder coords
                    Maybe: Increment by CV offsets
                    Vacuum on
                    Lower Z
                    Up Z
                */

                
                rovePnP.setState(ORIENT);
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

                rovePnP.setState(PLACE);
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
                
                state_t next_state = rovePnP.advanceComponent();
                rovePnP.setState(next_state);

                break;
            }
            case PAUSE: {
                cout << "FC: Pause state" << endl;

                //Wait for JSON to update
                rovePnP.setState(rovePnP.getPreviousState());
                break;
            }
            case ERROR: {
                cout << "FC: Error state" << endl;
                
                //Let QT app know
                rovePnP.handleError();
                break;
            }
            case RELOAD: {
                cout << "FC: Reload state:  P: " << rovePnP.getCurrentComponent().package << "  V: " << rovePnP.getCurrentComponent().value << "  Tape: " << rovePnP.getCurrentCutTape().width << endl;

                /*
                    Tell user which new cuttape to load
                    Wait for user to reload and go
                */
                
                rovePnP.setState(PICK);
                break;
            }
            case MANUAL: {
                cout << "FC: Manual state" << endl;

                /* Wait for user to finish */

                rovePnP.setState(rovePnP.getPreviousState());
                break;
            }
            case STOP:
                break;
            
        }

        /* 
            Poll from app interface, set state 
        */

        if( !rovePnP.isOK() ) rovePnP.setState(ERROR);

    }

    cout << "FC: Stop state" << endl;
    cout << "Placed: " << comp_count << endl;


    rovePnP.grbl.comm.closeComm();

    return 0;
}

void FC_msSleep(long int dur)
{
    this_thread::sleep_for(chrono::milliseconds(dur));
}