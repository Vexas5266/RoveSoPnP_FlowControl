
#include "grbl.hpp"

GRBL::GRBL(const char* commPort)
{
#if (INIT_COMM)
    // Start comm, fill csv
    std::cout << "Init Comm..." << std::endl;
    if (comm.setupComm(commPort) == false)
    {
        std::cout << "COM SETUP FAILED" << std::endl;
        return;
    }
#else
    return;
#endif

    // Flush startup
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    comm.readLine();    // Flush return
    std::cout << "GRBL Startup:  ";
    comm.readLine();    // Startup

    // Init GRBL
    std::cout << "GRBL Initializing..." << std::endl;
    comm.writeLine("?");
    std::cout << "Startup Status: ";
    comm.readLine();    // Status
    comm.readLine();    // Flush ok
}

// Works
GRBL_STATUS GRBL::pollStatus()
{
    // Ask GRBL for a status report
    comm.writeLine("?");
    if (!GRBL_FAST_MODE)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::string resp = comm.readLine();    // Get response

    if (waitForCommand() != GRBL_OK)
        return GRBL_STATUS::ERROR;

    if (resp.find("Idle") != std::string::npos)
        return GRBL_STATUS::IDLE;
    if (resp.find("Run") != std::string::npos)
        return GRBL_STATUS::BUSY;

    return GRBL_STATUS::ERROR;
}

bool GRBL::isBusy()
{
    if (pollStatus() == GRBL_STATUS::BUSY)
        return true;
    return false;
}

// Works
bool GRBL::waitForCommand()
{
    bool ok              = true;

    std::string response = comm.readLine();
    if (response != "ok")
        ok = false;

    return ok;
}

// Works
bool GRBL::sendCommand(std::string cmd_g)
{
    bool ok = true;
    comm.writeLine(cmd_g);
    if (!GRBL_FAST_MODE)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ok = waitForCommand();

    return ok;
}
