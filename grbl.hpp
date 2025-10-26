#ifndef GRBLH
#define GRBLH

#include "comm.hpp"
#include <thread>
#include <iostream>

#define TIMEOUT_E_S 15 //seconds

class GRBL {
    private:
    public:
        bool isIdle();
        bool waitForMotion();
        bool waitForCommand();
        Comm comm;

};

#endif /* GRBL_H */