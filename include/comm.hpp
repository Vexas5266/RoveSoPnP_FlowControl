#ifndef COMM_H
#define COMM_H

#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <thread>
#include <unistd.h>

#define BAUD           B115200
#define SERIAL_TIMEOUT 15

#define EN_ECHO   false
#define INIT_COMM true

class Comm
{
    private:
        int m_fd;

    public:
        bool setupComm(const char* portName);
        std::string readLine();
        void writeLine(const std::string& s);
        int getFD();
        void closeComm();
};

#endif /* COMM_H */
