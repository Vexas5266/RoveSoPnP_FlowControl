
#include "grbl.hpp"

bool GRBL::isIdle() 
{
    // Ask GRBL for a status report
    write(comm.getFD(), "?\n", 2);
    this_thread::sleep_for(chrono::milliseconds(100));
    string resp = comm.readLine();
    return resp.find("Idle") != string::npos;
}

bool GRBL::waitForMotion()
{
    bool ok = true;
    //Wait for GRBL to be idle
    int timeout = TIMEOUT_E_S;
    while ((!isIdle()) && (timeout > 0)) {
        cout << "Running..." << endl;
        timeout--;
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
    if (timeout <= 0) {
        cout << "ERROR: MOTION TIMED OUT" << endl;
        ok = false;
    }

    cout << "Motion complete." << endl;
    return ok;
}

bool GRBL::waitForCommand()
{
    //Wait for ok response
    bool ok = true;

    int timeout = TIMEOUT_E_S;
    while (timeout > 0)
    {
        string response = comm.readLine();
        if (response == "ok") break;
        if (!response.empty()) {
            cout << "GRBL: " << response << endl;
            ok = false;
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(1000));
        timeout--;
    }
    if (timeout <= 0) {
        cout << "ERROR: CMND SEND TIMED OUT" << endl;
        ok = false;
    }

    return ok;
}

void Comm::closeComm()
{
    close(m_fd);
}