#include "grbl.hpp"
#include <sstream>

GRBL::GRBL() {}

GRBL::~GRBL()
{
    disconnect();
}

void GRBL::openPort(const char* portName)
{
    closePort();

    m_fd = open(portName, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_fd < 0)
    {
        // Openning port failed
        std::cout << "[COM] Port opening failed. " << std::endl;
        return;
    }

    // Configure port
    struct termios tty{};
    if (tcgetattr(m_fd, &tty) != 0)
    {
        std::cerr << "Error from tcgetattr" << std::endl;
        closePort();
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
    }

    std::cout << "[COM] Port opened on: " << portName << std::endl;
}

void GRBL::closePort()
{
    if (m_fd >= 0)
    {
        close(m_fd);
        std::cout << "[COM] Port closed on FD: " << m_fd << std::endl;
        m_fd = -1;
    }
}

void GRBL::disconnect()
{
    closePort();
    std::cout << "[COM] Device intentionally disconnected." << std::endl;
}

std::string GRBL::readLine()
{
    if (m_fd < 0)
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
            {
                read(m_fd, &c, 1);    // Eat \n
                break;
            }
            if (c != '\r')
                line += c;
        }
        else if (timeout == 0)
        {
            std::cout << "[COM] Buffer empty n: " << n << std::endl;
            return "";
        }
        else if (n < 0)
        {
            timeout--;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        else if (n == 0)
        {
            std::cout << "[COM] Disconnected n: " << n << std::endl;
            closePort();
            return "";
        }
    }

    if (VERBOSE_SERIAL)
        std::cout << "[RX] " << line << std::endl;
    if (m_logCallback)
        m_logCallback("RX", line);

    return line;
}

void GRBL::writeLine(const std::string& s)
{
    if (m_fd < 0)
        return;

    std::string out = s + "\n";
    int n           = write(m_fd, out.c_str(), out.size());
    if (n < 0)
    {
        std::cerr << "[COM] Device accidentally disconnected W" << std::endl;
        closePort();
        return;
    }

    if (m_logCallback)
        m_logCallback("TX", s);
    if (VERBOSE_SERIAL)
        std::cout << "[TX] " << s << std::endl;
}

bool GRBL::connect(std::string portName)
{
    openPort(portName.c_str());
    if (m_fd < 0)
        return false;

    // Flush startup
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    readLine();    // Flush return
    std::cout << "GRBL Startup:  ";
    readLine();    // Startup
    readLine();    // Unlock message

    // Init GRBL
    std::cout << "GRBL Initializing..." << std::endl;
    writeLine("?");
    std::cout << "Startup Status: ";
    readLine();    // Status
    readLine();    // Flush ok

    // Add $ setup commands

    return true;
}

bool GRBL::checkConnection()
{
    // if (!m_commConnected)
    // {
    //     if (m_explicitDisconnect)
    //     {
    //         return false;    // Don't auto-reconnect if the user manually disconnected
    //     }
    //     return connect(m_commPort);
    // }
    return true;
}

GRBL_STATUS GRBL::pollStatus()
{
    writeLine("?");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::string resp = readLine();

    if (waitForCommand() != GRBL_OK)
        return GRBL_STATUS::ERROR;

    if (resp.find("Idle") != std::string::npos)
        return GRBL_STATUS::IDLE;
    if (resp.find("Run") != std::string::npos)
        return GRBL_STATUS::BUSY;
    if (resp.find("Alarm") != std::string::npos)
        return GRBL_STATUS::ALARM;

    return GRBL_STATUS::ERROR;
}

bool GRBL::isBusy()
{
    if (pollStatus() == GRBL_STATUS::BUSY)
        return true;

    return false;
}

grbl_position_t GRBL::getMachinePosition()
{
    grbl_position_t ret = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    writeLine("?");
    std::string resp = readLine();

    // Safety check: ensure string isn't empty and actually contains a position report
    if (resp.empty() || resp.find("MPos:") == std::string::npos)
        return ret;

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

    std::cout << "X:" << ret.x << " Y:" << ret.y << " Z:" << ret.z << " A:" << ret.a << " B:" << ret.b << " C:" << ret.c << std::endl;

    return ret;
}

bool GRBL::waitForCommand()
{
    std::string response = readLine();

    if (response == "ok")
        return true;

    if (response.find("MSG") != std::string::npos)
    {
        std::cout << "itsa msg:";
        if (readLine() == "ok")
            return true;
    }

    return false;
}

bool GRBL::sendCommand(std::string cmd_g)
{
    bool ok = true;
    writeLine(cmd_g);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ok = waitForCommand();

    return ok;
}
