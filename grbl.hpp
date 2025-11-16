#ifndef GRBLH
#define GRBLH

#include "comm.hpp"
#include <thread>
#include <iostream>

#define GRBL_TIMEOUT 60 //seconds
#define GRBL_OK true

#define EN_GRBL_STAT false

typedef enum {
    IDLE_G,
    RUN_G,
    ERROR_G,
    CNT_G
} GRBL_status_t;

class GRBL {
    private:
    public:
        GRBL_status_t pollStatus();
        bool waitForMotion();
        bool waitForCommand();
        bool sendCommand(string cmd_g);
        bool sendMotion(string motion_g);
        void init();

        Comm comm;

};

#endif /* GRBL_H */