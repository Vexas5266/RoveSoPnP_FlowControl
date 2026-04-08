
#include "gantry.hpp"
#include <thread>

// Untested
void Gantry::setGlobalPosition(float x, float y)
{
    m_grbl->sendCommand("G90 G0 X" + std::to_string(x) + " Y" + std::to_string(y));
}

// Untested
void Gantry::setHeadHeight(float z)
{
    m_grbl->sendCommand("G90 G0 Z" + std::to_string(z));
}

// Untested
void Gantry::home()
{
    m_grbl->sendCommand("$H", 60000);
    m_grbl->sendCommand("G92 X0 Y0 Z0");
}
