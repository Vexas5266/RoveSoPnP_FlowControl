#pragma once

#include <chrono>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>

#define SERIAL_TIMEOUT 10
#define EN_ECHO        0

class Comm
{
    public:
        bool INIT_COMM = true;

        bool setupComm(const char* portName);
        std::string readLine();
        void writeLine(const std::string& s);
        int getFD();
        void closeComm();

        // Callback definition for logging GRBL commands
        using LogCallback = std::function<void(const std::string& dir, const std::string& msg)>;
        void setLogCallback(LogCallback cb);

    private:
        int m_fd                  = -1;
        LogCallback m_logCallback = nullptr;
};
