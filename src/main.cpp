#include <math.h>

#include <thread>
#include <chrono>

#include "pnp.hpp"

using namespace std;

#define PNP_FILE "../board/ArmBoard_Hardware-all-pos.csv"
#define COM_PORT "/dev/tty.wchusbserial130"

void FC_msSleep(long int dur); //in seconds

// Need to set initial offsets from testing for Z and A axes

int main() {

    /*
    
        Ask User:
        CSV File
        Com port

    */

    PnP rovePnP(COM_PORT, PNP_FILE);

    /*
        Parse CSV and fill in components, add to placement map
        Iterate through each tuple in placement_map
        Compare package to look up packages
            set cuttape config
        else 
            ask user to input or remove from placement map
    */

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

        rovePnP.tickStateMachine();

    }

    cout << "FC: Stop state" << endl;
    cout << "Placed: " << comp_count << endl;

    return 0;
}

void FC_msSleep(long int dur)
{
    this_thread::sleep_for(chrono::milliseconds(dur));
}