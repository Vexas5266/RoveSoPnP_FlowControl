/*

== Coordinate System ==
Z=0: Bed
Origin: Top left corner
A: Rotation axis

Z+
/\
\/
Z-
 O X- <--------> X+
Y+
/\
||
||
\/
Y-

== G-Code Refrence

Fn                  (Set Feed Rate)
G0 Xn Yn Zn         (Rapid)
G1 Xn Yn Zn         (Feed Rate)
G28 X Y             (Home)
G90                 (absolute mode)
G91                 (incremental mode)
G92 Xn Yn Zn        (Set current pos to given coords)
G94                 (Units per minute mode)

*/

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>

#include <fcntl.h>
#include <unistd.h> 
#include <termios.h>

using namespace std;

#define BAUD B115200
#define SPEED 300

typedef struct {
    string ref;
    string value;
    string package;
    float posX;
    float posY;
    float rotation;
} component_t;

string parseItemString(stringstream& s);
float parseItemFloat(stringstream &s);
int setupComm();

vector<component_t> placement_list;
ifstream file("ArmBoard_Hardware-all-pos.csv");

// Need to set initial offsets from testing for Z and A axes
const string init_g = "G20 G94 F" + SPEED;
const string home_g = "G90 G0 Z0.5\nG28 X Y\nG92 X0 Y0";

int main() {

    int fd = setupComm();

    string line;
    getline(file, line, '\n');
    while (getline(file, line, '\n')) {
        stringstream ss(line);
        component_t component = {
            parseItemString(ss), //ref
            parseItemString(ss), //value
            parseItemString(ss), //package
            parseItemFloat(ss), //posX
            parseItemFloat(ss), //posY
            parseItemFloat(ss), //rotation
        };

        placement_list.push_back(component);

    }

    close(fd);
} 

string parseItemString(stringstream &s)
{
    string data;
    getline(s, data, ',');
    data = data.substr(1, data.length() - 2);
    cout << data << endl;
    return data;
}

float parseItemFloat(stringstream &s)
{
    string data;
    getline(s, data, ',');
    cout << data << endl;
    return stof(data);
}

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