#include "grbl.hpp"
#include <QString>
#include <algorithm>
#include <iostream>

#ifndef INIT_COMM
#define INIT_COMM 1
#endif

// Helper to delay without blocking the UI Thread
static void delay_ms(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
}

GRBL::GRBL(const char* commPort, QObject* parent) : QObject(parent), m_commPort(commPort)
{
    m_serial = new QSerialPort(this);
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

    m_serial->setPortName(QString::fromStdString(portName));
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serial->open(QIODevice::ReadWrite))
    {
        m_connected = true;
        return true;
    }

    std::cerr << "GRBL: Failed to open port " << portName << std::endl;
    m_connected = false;
    return false;
}

void GRBL::closePort()
{
    if (m_serial->isOpen())
    {
        m_serial->close();
    }
    m_connected = false;
}

void GRBL::disconnect()
{
    closePort();
    m_explicitDisconnect = true;
}

std::string GRBL::readLine(int timeout_ms)
{
    if (!m_connected || !m_serial->isOpen())
        return "";

    // 1. Check if we already have a full line buffered
    int newlineIdx = m_readBuffer.indexOf('\n');
    if (newlineIdx != -1)
    {
        QByteArray lineData = m_readBuffer.left(newlineIdx);
        m_readBuffer.remove(0, newlineIdx + 1);

        std::string line = lineData.toStdString();
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        if (!line.empty() && m_logCallback)
            m_logCallback("RX", line);
        return line;
    }

    // 2. Wait for hardware data asynchronously
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    bool hasLine = false;

    // Listen for new data. Using fully qualified QObject::connect to avoid name collisions
    auto connection = QObject::connect(m_serial,
                                       &QSerialPort::readyRead,
                                       [&]()
                                       {
                                           m_readBuffer.append(m_serial->readAll());
                                           if (m_readBuffer.contains('\n'))
                                           {
                                               hasLine = true;
                                               loop.quit();
                                           }
                                       });

    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(timeout_ms);
    // This blocks the execution here so the C++ API remains synchronous,
    // BUT allows Qt to repaint the GUI and run timers.
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    QObject::disconnect(connection);

    // 3. Process result after the event loop ends
    newlineIdx = m_readBuffer.indexOf('\n');
    if (newlineIdx != -1)
    {
        QByteArray lineData = m_readBuffer.left(newlineIdx);
        m_readBuffer.remove(0, newlineIdx + 1);

        std::string line = lineData.toStdString();
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        if (!line.empty() && m_logCallback)
            m_logCallback("RX", line);
        return line;
    }

    return "";    // Timeout occurred
}

void GRBL::writeLine(const std::string& s)
{
    if (!m_connected || !m_serial->isOpen())
        return;

    std::string out = s + "\n";
    m_serial->write(out.c_str(), out.size());

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

    std::cout << "Init Comm on " << m_commPort << "..." << std::endl;
    if (!openPort(m_commPort.c_str()))
    {
        return false;
    }

    std::cout << "Waiting for GRBL bootloader..." << std::endl;

    auto start         = std::chrono::steady_clock::now();
    bool found_welcome = false;

    while (true)
    {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > 3000)
            break;

        std::string line = readLine(250);
        if (line.find("Grbl") != std::string::npos)
        {
            found_welcome = true;
            break;
        }
    }

    std::cout << "Flushing buffers..." << std::endl;
    while (true)
    {
        std::string flush = readLine(100);
        if (flush.empty())
            break;
    }

    if (!found_welcome)
    {
        std::cerr << "Warning: Did not see GRBL welcome message. Forcing sync..." << std::endl;
        writeLine("");
        waitForCommand(1000);
    }

    // Send GRBL config commands to ensure it's in a known state.
    std::cout << "Sending configuration commands..." << std::endl;
    const char* config_cmds[] = {"$0=10",         "$1=0",         "$2=0",         "$3=10",        "$4=0",         "$5=0",         "$6=0",         "$10=1",
                                 "$11=0.020",     "$12=0.002",    "$13=0",        "$20=0",        "$21=1",        "$22=1",        "$23=35",       "$24=25.000",
                                 "$25=250.000",   "$26=250",      "$27=5.000",    "$30=99",       "$31=550",      "$32=0",        "$100=79.877",  "$101=80.141",
                                 "$102=73.665",   "$103=1.111",   "$104=8.889",   "$105=79.877",  "$110=750.000", "$111=750.000", "$112=750.000", "$113=1440.000",
                                 "$114=1440.000", "$115=750.000", "$120=50.000",  "$121=50.000",  "$122=50.000",  "$123=50.000",  "$124=50.000",  "$125=50.000",
                                 "$130=400.000",  "$131=200.000", "$132=200.000", "$133=360.000", "$134=180.000", "$135=400.000"};

    for (const char* cmd : config_cmds)
    {
        // Give each command up to 1000ms to acknowledge with 'ok'
        if (!sendCommand(cmd, 1000))
        {
            std::cerr << "Warning: Failed to confirm configuration: " << cmd << std::endl;
        }
    }

    std::cout << "GRBL Successfully Initialized." << std::endl;
    return true;
}

bool GRBL::checkConnection()
{
    if (!m_connected)
    {
        if (m_explicitDisconnect)
            return false;
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
        delay_ms(100);

    std::string resp;

    while (true)
    {
        resp = readLine(1000);
        if (resp.empty() || resp.find("error") != std::string::npos)
            return GRBL_STATUS::ERROR;

        if (resp.find("<") != std::string::npos)
            break;
    }

    waitForCommand(500);

    if (resp.find("Idle") != std::string::npos)
        return GRBL_STATUS::IDLE;
    if (resp.find("Run") != std::string::npos || resp.find("Home") != std::string::npos)
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

    std::string resp;
    while (true)
    {
        resp = readLine(1000);
        if (resp.empty() || resp.find("error") != std::string::npos)
            return ret;

        if (resp.find("<") != std::string::npos)
            break;
    }

    waitForCommand(500);

    if (resp.empty() || resp.find("MPos:") == std::string::npos)
        return ret;

    try
    {
        std::istringstream tokenStream(resp);
        std::string output;
        std::getline(tokenStream, output, ':');

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
        // Suppress parsing exceptions
    }

    return ret;
}

bool GRBL::waitForCommand(int timeout_ms)
{
    if (!checkConnection())
        return false;

    auto start = std::chrono::steady_clock::now();

    while (true)
    {
        auto now      = std::chrono::steady_clock::now();
        int elapsed   = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        int remaining = timeout_ms - elapsed;

        if (remaining <= 0)
            return false;

        std::string response = readLine(remaining);

        if (response.empty())
            return false;

        if (response == "ok")
            return true;

        if (response.find("error") != std::string::npos)
            return false;
    }
}

bool GRBL::sendCommand(std::string cmd_g, int timeout_ms)
{
    if (!checkConnection())
        return false;

    writeLine(cmd_g);

    if (!GRBL_FAST_MODE)
        delay_ms(50);

    return waitForCommand(timeout_ms);
}

void GRBL::feedHold()
{
    if (!m_connected || !m_serial->isOpen())
        return;

    // Send the ! character immediately without a newline
    m_serial->write("!");

    if (m_logCallback)
    {
        m_logCallback("TX", "!");
    }
}

void GRBL::cycleStart()
{
    if (!m_connected || !m_serial->isOpen())
        return;

    // Send the ~ character immediately without a newline
    m_serial->write("~");

    if (m_logCallback)
    {
        m_logCallback("TX", "~");
    }
}
