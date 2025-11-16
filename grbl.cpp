
#include "grbl.hpp"

//Works
GRBL_status_t GRBL::pollStatus() 
{
    // Ask GRBL for a status report
    comm.writeLine("?");
    this_thread::sleep_for(chrono::milliseconds(100));
    string resp = comm.readLine(); //Get response

    if (waitForCommand() != GRBL_OK) return ERROR_G;

    if (resp.find("Idle") != string::npos) return IDLE_G;
    if (resp.find("Run") != string::npos) return RUN_G;

    return ERROR_G;
}

//Works
bool GRBL::waitForMotion()
{
    bool ok = true;
    //Wait for GRBL to be idle
    int timeout = GRBL_TIMEOUT;
    GRBL_status_t status;

    do
    {
        status = pollStatus();

        timeout--;
        this_thread::sleep_for(chrono::milliseconds(1000));
    } while ((status == RUN_G) && (timeout > 0));
    
    if (timeout <= 0) {
        #if (EN_GRBL_STAT)
            cout << "Motion: Timed out" << endl;
        #endif
        ok = false;
    }

    if (status == ERROR_G) ok = false;

    #if (EN_GRBL_STAT)
        if (ok == GRBL_OK) cout << "Motion: Complete" << endl;
    #endif

    return ok;
}

//Works
bool GRBL::waitForCommand()
{
    bool ok = true;

    string response = comm.readLine();
    if (response != "ok") ok = false;

    return ok;
}

//Works
bool GRBL::sendCommand(string cmd_g)
{
    bool ok = true;
    comm.writeLine(cmd_g);
    this_thread::sleep_for(chrono::milliseconds(50));
    ok = waitForCommand();

    return ok;
}

//Works
bool GRBL::sendMotion(string motion_g)
{
    bool ok = true;

    ok = sendCommand(motion_g);
    this_thread::sleep_for(chrono::milliseconds(50));
    if (ok == GRBL_OK) ok = waitForMotion();

    return ok;
}

void GRBL::init()
{
    // const string prep_g = "[Ctrl+X]]\nS1000\n$#";
    // const string init_g = "G21\nG94";
    // const string home_g = "G90\nG1 Z0.5\nG28 X Y\nG92 X0 Y0";

}