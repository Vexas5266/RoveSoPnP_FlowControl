#include "comm.hpp"

void Comm::setLogCallback(LogCallback cb)
{
    m_logCallback = cb;
}

void Comm::connect(const char* portName)
{
    if (m_connected)
        disconnect();

    m_fd = open(portName, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_fd < 0)
    {
        std::cerr << "Error opening " << portName << std::endl;
        m_connected = false;
        return;
    }

    // Configure port
    struct termios tty{};
    if (tcgetattr(m_fd, &tty) != 0)
    {
        std::cerr << "Error from tcgetattr" << std::endl;
        disconnect();
        return;
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
        disconnect();
        return;
    }

    m_connected = true;
    return;
}

void Comm::disconnect()
{
    close(m_fd);
    m_connected = false;
}

std::string Comm::readLine()
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
            // Removed stdout spam for cleaner terminal
            line = "";
            disconnect();
            break;
        }
        else if (n == 0 || ((n < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)))
        {
            // no data available, sleep a bit and try again
            timeout--;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else
            break;
    }

    if (!line.empty())
    {
        // Emit the received line to the terminal UI
        if (m_logCallback)
        {
            m_logCallback("RX", line);
        }
    }

    return line;
}

void Comm::writeLine(const std::string& s)
{
    if (!m_connected)
        return;

    std::string out = s + "\n";
    int n           = write(m_fd, out.c_str(), out.size());
    if (n < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            std::cerr << "Comm: Hardware write error (Disconnect detected). Errno: " << errno << std::endl;
            disconnect();
            return;
        }
    }

    // Emit the sent line to the terminal UI
    if (m_logCallback)
    {
        m_logCallback("TX", s);
    }
}
