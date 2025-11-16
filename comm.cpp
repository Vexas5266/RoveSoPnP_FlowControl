
#include "comm.hpp"

bool Comm::setupComm()
{

    const char* portName = "/dev/tty.usbserial-140"; // adjust to match your port
    m_fd = open(portName, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_fd < 0) {
        std::cerr << "Error opening " << portName << std::endl;
        return false;
    }

    // Configure port
    struct termios tty{};
    if (tcgetattr(m_fd, &tty) != 0) {
        std::cerr << "Error from tcgetattr" << std::endl;
        close(m_fd);
        return false;
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    tty.c_iflag &= ~IGNBRK;                         // disable break processing
    tty.c_lflag = 0;                                // no signaling chars, no echo
    tty.c_oflag = 0;                                // no remapping, no delays
    tty.c_cc[VMIN]  = 1;                            // read at least 1 char
    tty.c_cc[VTIME] = 50;                            // timeout 0.1 s

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // shut off xon/xoff ctrl
    tty.c_cflag |= (CLOCAL | CREAD);                // ignore modem controls, enable reading
    tty.c_cflag &= ~(PARENB | PARODD);              // no parity
    tty.c_cflag &= ~CSTOPB;                         // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                        // no hardware flow control

    if (tcsetattr(m_fd, TCSANOW, &tty) != 0) {
        std::cerr << "Error from tcsetattr" << std::endl;
        close(m_fd);
        return false;
    }

    return true;

}

string Comm::readLine() {
    string line;
    char c;
    int timeout = SERIAL_TIMEOUT;
    while (true) {
        int n = read(m_fd, &c, 1);
        if (n > 0) {
            if (c == '\n') break;
            if (c != '\r') line += c;
        } else if (timeout == 0) {
            cout << "Comm: Timed out" << endl;
            line = "";
            //SEND TO QT
            break;
        } else if (n == 0 || ((n < 0) && (errno == EAGAIN || errno == EWOULDBLOCK))) {
            // no data available, sleep a bit and try again
            timeout--;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } else break;
    }

    if (!line.empty()) {
        #if (EN_ECHO)
            cout << "GRBL: " << line << endl;
        #endif
    }
    
    return line;
}

void Comm::writeLine(const string &s) 
{
    string out = s + "\n";
    int n = write(m_fd, out.c_str(), out.size());
    if (n < 0)
    {
        cout << "Comm: Write error " << endl;
        //SEND TO QT
    }
    #if (EN_ECHO)
        cout << "Sent: " << s << endl;
    #endif
}

int Comm::getFD()
{
    return m_fd;
}

void Comm::closeComm()
{
    close(m_fd);
}