#ifndef GRBLH
#define GRBLH

#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

// Qt Includes for Async Serial Communication
#include <QByteArray>
#include <QEventLoop>
#include <QObject>
#include <QSerialPort>
#include <QTimer>

#define GRBL_OK           true
#define GRBL_FAST_MODE    true
#define EN_GRBL_STAT      false
#define SERIAL_TIMEOUT_MS 5000

enum class GRBL_STATUS
{
    IDLE,
    BUSY,
    ERROR,
    COUNT
};

struct grbl_position_t
{
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float a = 0.0f;
        float b = 0.0f;
        float c = 0.0f;
};

class GRBL : public QObject
{
        Q_OBJECT
    public:
        using LogCallback = std::function<void(const std::string& dir, const std::string& msg)>;

        GRBL(const char* commPort = "/dev/ttyACM0", QObject* parent = nullptr);
        ~GRBL();

        bool connect(std::string portName = "");
        void disconnect();

        bool checkConnection();

        GRBL_STATUS pollStatus();
        bool isBusy();
        grbl_position_t getMachinePosition();

        bool waitForCommand(int timeout_ms = SERIAL_TIMEOUT_MS);
        bool sendCommand(std::string cmd_g, int timeout_ms = SERIAL_TIMEOUT_MS);

        void feedHold();
        void cycleStart();

        bool isConnected() const { return m_connected; }

        void setLogCallback(LogCallback cb) { m_logCallback = cb; }

    private:
        std::string m_commPort;
        QSerialPort* m_serial;
        bool m_connected          = false;
        bool m_explicitDisconnect = false;
        LogCallback m_logCallback = nullptr;

        QByteArray m_readBuffer;

        bool openPort(const char* portName);
        void closePort();
        std::string readLine(int timeout_ms = SERIAL_TIMEOUT_MS);
        void writeLine(const std::string& s);
};

#endif /* GRBLH */
