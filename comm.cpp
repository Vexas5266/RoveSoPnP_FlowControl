
#include "comm.hpp"

int setupComm()
{
    int fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);

    if (fd < 0) {
        perror("openSerial");
        return -1;
    }
    
    struct termios tty;

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return -1;
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

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return -1;
    }

    return fd;
}

string readLine(int fd) {
    string line;
    char c;
    while (true) {
        int n = read(fd, &c, 1);
        if (n > 0) {
            if (c == '\n') break;
            if (c != '\r') line += c;
        } else {
            break;
        }
    }
    return line;
}

void writeLine(int fd, const string &s) 
{
    string out = s + "\n";
    write(fd, out.c_str(), out.size());
}