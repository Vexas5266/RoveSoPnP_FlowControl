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

class Comm
{
    public:
        void connect(const char* portName);
        void disconnect();
        std::string readLine();
        void writeLine(const std::string& s);

        int getFD() { return m_fd; }

        bool isConnected() { return m_connected; }

        // Callback definition for logging GRBL commands
        using LogCallback = std::function<void(const std::string& dir, const std::string& msg)>;
        void setLogCallback(LogCallback cb);

    private:
        int m_fd                  = -1;
        LogCallback m_logCallback = nullptr;
        bool m_connected          = false;
};
