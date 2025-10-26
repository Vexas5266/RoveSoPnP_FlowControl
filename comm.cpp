
#include "comm.hpp"

void Comm::setupComm()
{
    m_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);

    if (m_fd < 0) {
        perror("openSerial");
        return;
    }
    
    struct termios tty;

    if (tcgetattr(m_fd, &tty) != 0) {
        perror("tcgetattr");
        return;
    }

    cfsetospeed(&tty, BAUD);
    cfsetispeed(&tty, BAUD);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;            // no signaling chars, no echo
    tty.c_oflag = 0;            // no remapping, no delays
    tty.c_cc[VMIN]  = 0;        // read doesn't block
    tty.c_cc[VTIME] = 5;        // 0.5s read timeout

    tty.c_cflag |= (CLOCAL | CREAD); // Enable receiver and ignore modem control lines
    tty.c_cflag &= ~(PARENB | PARODD); // No parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tty.c_cflag &= ~CSIZE; 
    tty.c_cflag |= CS8; // 8 data bits

    if (tcsetattr(m_fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return;
    }

    return;
}

string Comm::readLine() {
    string line;
    char c;
    while (true) {
        int n = read(m_fd, &c, 1);
        if (n > 0) {
            if (c == '\n') break;
            if (c != '\r') line += c;
        } else {
            break;
        }
    }
    return line;
}

void Comm::writeLine(const string &s) 
{
    string out = s + "\n";
    write(m_fd, out.c_str(), out.size());
}

int Comm::getFD()
{
    return m_fd;
}