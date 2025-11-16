#ifndef COMM_H
#define COMM_H

#include <string>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/ioctl.h>
#include <thread>

using namespace std;

#define BAUD B115200
#define SERIAL_TIMEOUT 15

#define EN_ECHO false

const string speed = "300";

class Comm {
    private:
        int m_fd;

    public:
        bool setupComm();
        string readLine();
        void writeLine(const string &s);
        bool pollStatus();
        int getFD();
        void closeComm();

};

#endif /* COMM_H */