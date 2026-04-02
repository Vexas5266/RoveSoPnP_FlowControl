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
#define GRBL_FAST_MODE true
#define EN_GRBL_STAT   false
#define SERIAL_TIMEOUT 10

enum class GRBL_STATUS
{
    IDLE,
    BUSY,
    ERROR,
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

        GRBL(const char* commPort = "/dev/ttyACM0");
        ~GRBL();

        bool connect(std::string portName = "");    // Public connect with optional port override
        void disconnect();                          // Public explicit disconnect

        bool checkConnection();                     // Reconnects if the line dropped

        GRBL_STATUS pollStatus();
        bool isBusy();
        grbl_position_t getMachinePosition();
        bool waitForCommand();
        bool sendCommand(std::string cmd_g);

        bool isConnected() const { return m_connected; }

        void setLogCallback(LogCallback cb) { m_logCallback = cb; }

    private:
        // State variables
        std::string m_commPort;
        int m_fd                  = -1;
        bool m_connected          = false;
        bool m_explicitDisconnect = false;
        LogCallback m_logCallback = nullptr;

        // Internal communication methods
        bool openPort(const char* portName);
        void closePort();
        std::string readLine();
        void writeLine(const std::string& s);
};

#endif /* GRBL_H */
