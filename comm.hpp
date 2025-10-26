#ifndef COMM_H
#define COMM_H

#include <string>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

#define BAUD B115200

class Comm {
    private:
        int m_fd;

    public:
        void setupComm();
        string readLine();
        void writeLine(const string &s);
        bool isIdle();
        int getFD();
        void closeComm();

};

#endif /* COMM_H */