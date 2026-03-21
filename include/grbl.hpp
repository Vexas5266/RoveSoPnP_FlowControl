#ifndef GRBLH
#define GRBLH

#include "comm.hpp"
#include <iostream>
#include <thread>

#define GRBL_TIMEOUT       60      // seconds
#define GRBL_WAIT_INTERVAL 1000    // ms
#define GRBL_OK            true
#define GRBL_FAST_MODE     true

#define EN_GRBL_STAT false

typedef enum
{
    IDLE_G,
    RUN_G,
    ERROR_G,
    CNT_G
} GRBL_status_t;

class GRBL
{
    private:
    public:
        GRBL();
        GRBL_status_t pollStatus();
        bool waitForMotion();
        bool waitForCommand();
        bool sendCommand(std::string cmd_g);
        bool sendMotion(std::string motion_g);
        void init();

        bool isBusy() { return pollStatus() == RUN_G; }

        Comm comm;
};

#endif /* GRBL_H */
