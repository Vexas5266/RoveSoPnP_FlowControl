#ifndef COMM_H
#define COMM_H

#include <string>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

#define BAUD B115200

int setupComm();
string readLine(int fd);
void writeLine(int fd, const string &s);
bool isIdle(int fd);

#endif /* COMM_H */