#ifndef GRBLH
#define GRBLH

#include "comm.hpp"
#include <iostream>
#include <thread>

#define GRBL_OK        true
#define GRBL_FAST_MODE true

#define EN_GRBL_STAT false

enum class GRBL_STATUS
{
    IDLE,
    BUSY,
    ERROR,
    COUNT
};

class GRBL
{
    private:
    public:
        GRBL(const char* commPort);
        GRBL_STATUS pollStatus();
        bool waitForCommand();
        bool sendCommand(std::string cmd_g);

        Comm comm;
};

#endif /* GRBL_H */
