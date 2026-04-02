#include "grbl.hpp"
#include <sstream>

// Prevent build errors if CMake doesn't provide this definition
#ifndef INIT_COMM
#define INIT_COMM 1
#endif

GRBL::GRBL(const char* commPort) : m_commPort(commPort)
{
    connect();
}

GRBL::~GRBL()
{
    disconnect();
}

bool GRBL::openPort(const char* portName)
{
    if (m_connected)
        closePort();

    m_fd = open(portName, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_fd < 0)
    {
        // Suppress std::cerr spam during disconnect polling
        m_connected = false;
        return false;
    }

    // Configure port
    struct termios tty{};
    if (tcgetattr(m_fd, &tty) != 0)
    {
        std::cerr << "Error from tcgetattr" << std::endl;
        closePort();
        return false;
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;    // 8-bit chars
    tty.c_iflag &= ~IGNBRK;                        // disable break processing
    tty.c_lflag     = 0;                           // no signaling chars, no echo
    tty.c_oflag     = 0;                           // no remapping, no delays
    tty.c_cc[VMIN]  = 1;                           // read at least 1 char
    tty.c_cc[VTIME] = 50;                          // timeout 0.1 s

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);        // shut off xon/xoff ctrl
    tty.c_cflag |= (CLOCAL | CREAD);               // ignore modem controls, enable reading
    tty.c_cflag &= ~(PARENB | PARODD);             // no parity
    tty.c_cflag &= ~CSTOPB;                        // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                       // no hardware flow control

    if (tcsetattr(m_fd, TCSANOW, &tty) != 0)
    {
        std::cerr << "Error from tcsetattr" << std::endl;
        closePort();
        return false;
    }

    m_connected = true;
    return true;
}

void GRBL::closePort()
{
    if (m_fd >= 0)
    {
        close(m_fd);
        m_fd = -1;
    }
    m_connected = false;
}

void GRBL::disconnect()
{
    closePort();
    m_explicitDisconnect = true;
}

std::string GRBL::readLine()
{
    if (!m_connected)
        return "";

    std::string line;
    char c;
    int timeout = SERIAL_TIMEOUT;
    while (true)
    {
        int n = read(m_fd, &c, 1);
        if (n > 0)
        {
            if (c == '\n')
                break;
            if (c != '\r')
                line += c;
        }
        else if (timeout == 0)
        {
            line = "";
            closePort();
            break;
        }
        else if (n == 0 || ((n < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)))
        {
            // no data available, sleep a bit and try again
            timeout--;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else
        {
            // Actual read hardware fault detected
            closePort();
            break;
        }
    }

    if (!line.empty() && m_logCallback)
    {
        m_logCallback("RX", line);
    }

    return line;
}

void GRBL::writeLine(const std::string& s)
{
    if (!m_connected)
        return;

    std::string out = s + "\n";
    int n           = write(m_fd, out.c_str(), out.size());
    if (n < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            std::cerr << "GRBL: Hardware write error (Disconnect detected). Errno: " << errno << std::endl;
            closePort();
            return;
        }
    }

    if (m_logCallback)
    {
        m_logCallback("TX", s);
    }
}

bool GRBL::connect(std::string portName)
{
    m_explicitDisconnect = false;
    if (!portName.empty())
    {
        m_commPort = portName;
    }

#if (INIT_COMM)
    std::cout << "Init Comm on " << m_commPort << "..." << std::endl;
    if (!openPort(m_commPort.c_str()))
    {
        return false;
    }
#else
    return false;
#endif

    // Flush startup
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    readLine();    // Flush return
    std::cout << "GRBL Startup:  ";
    readLine();    // Startup

    // Init GRBL
    std::cout << "GRBL Initializing..." << std::endl;
    writeLine("?");
    std::cout << "Startup Status: ";
    readLine();    // Status
    readLine();    // Flush ok

    return true;
}

bool GRBL::checkConnection()
{
    if (!m_connected)
    {
        if (m_explicitDisconnect)
        {
            return false;    // Don't auto-reconnect if the user manually disconnected
        }
        return connect();
    }
    return true;
}

GRBL_STATUS GRBL::pollStatus()
{
    if (!checkConnection())
        return GRBL_STATUS::ERROR;

    writeLine("?");
    if (!GRBL_FAST_MODE)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::string resp = readLine();

    if (waitForCommand() != GRBL_OK)
        return GRBL_STATUS::ERROR;

    if (resp.find("Idle") != std::string::npos)
        return GRBL_STATUS::IDLE;
    if (resp.find("Run") != std::string::npos)
        return GRBL_STATUS::BUSY;

    return GRBL_STATUS::ERROR;
}

bool GRBL::isBusy()
{
    if (!checkConnection())
        return false;

    if (pollStatus() == GRBL_STATUS::BUSY)
        return true;

    return false;
}

grbl_position_t GRBL::getMachinePosition()
{
    grbl_position_t ret = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    if (!checkConnection())
        return ret;

    writeLine("?");
    std::string resp = readLine();

    // Safety check: ensure string isn't empty and actually contains a position report
    if (resp.empty() || resp.find("MPos:") == std::string::npos)
        return ret;

    try
    {
        std::istringstream tokenStream(resp);
        std::string output;
        std::getline(tokenStream, output, ':');    // Get up to MPos:

        std::getline(tokenStream, output, ',');    // X
        ret.x = std::stof(output);
        std::getline(tokenStream, output, ',');    // Y
        ret.y = std::stof(output);
        std::getline(tokenStream, output, ',');    // Z
        ret.z = std::stof(output);
        std::getline(tokenStream, output, ',');    // A
        ret.a = std::stof(output);
        std::getline(tokenStream, output, ',');    // B
        ret.b = std::stof(output);
        std::getline(tokenStream, output, '|');    // C
        ret.c = std::stof(output);
    }
    catch (...)
    {
        // Catch any std::invalid_argument or std::out_of_range exceptions from stof
        // If the serial data was garbled, we'll just safely return the default {0,0,0,0,0,0}
        // instead of crashing the program.
    }

    return ret;
}

bool GRBL::waitForCommand()
{
    if (!checkConnection())
        return false;

    // Loop until we get a definitive "ok", an "error", or a timeout
    while (true)
    {
        std::string response = readLine();

        if (response.empty())
            return false;    // Timeout or disconnect

        if (response == "ok")
            return true;

        if (response.find("error") != std::string::npos)
            return false;

        // If it's a [MSG:...] or something else, loop again and read the next line
    }
}

bool GRBL::sendCommand(std::string cmd_g)
{
    if (!checkConnection())
        return false;

    bool ok = true;
    writeLine(cmd_g);
    if (!GRBL_FAST_MODE)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ok = waitForCommand();

    return ok;
}
