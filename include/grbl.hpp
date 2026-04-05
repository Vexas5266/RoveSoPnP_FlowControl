#ifndef GRBLH
#define GRBLH

#include <chrono>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>

#define GRBL_OK        true
#define VERBOSE_SERIAL true
#define SERIAL_TIMEOUT 1000    // ms

enum class GRBL_STATUS
{
    IDLE,
    BUSY,
    ERROR,
    ALARM,
    COUNT
};

struct grbl_position_t
{
        float x;
        float y;
        float z;
        float a;
        float b;
        float c;
};

class GRBL
{
    public:
        // Callback definition for logging GRBL commands
        using LogCallback = std::function<void(const std::string& dir, const std::string& msg)>;

        GRBL();    // Must call connect() after constructing, and before usage
        ~GRBL();

        bool connect(std::string portName = "");    // Public connect with optional port override
        void disconnect();                          // Public explicit disconnect

        bool checkConnection();                     // Reconnects if the line dropped

        GRBL_STATUS pollStatus();
        bool isBusy();
        grbl_position_t getMachinePosition();
        bool waitForCommand();
        bool sendCommand(std::string cmd_g);

        void setLogCallback(LogCallback cb) { m_logCallback = cb; }

        std::string readLine();
        void writeLine(const std::string& s);    // Move back to private

    private:
        // State variables
        int m_fd                  = -1;
        LogCallback m_logCallback = nullptr;

        // Internal communication methods
        void openPort(const char* portName);
        void closePort();
};

#endif /* GRBL_H */
