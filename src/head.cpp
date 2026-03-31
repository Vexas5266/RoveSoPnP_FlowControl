
#include "head.hpp"

// Untested
void Head::increment(int degrees)
{
    if ((m_angle >= 720) || (m_angle <= -720))
        m_angle = 0;

    m_angle += degrees;

    m_grbl->sendCommand("G90 G0 A" + std::to_string(m_angle));
}

// Untested
void Head::vacuumOn()
{
    m_vacuum_state = VACUUM_STATE::ON;
    m_grbl->sendCommand("$32=0");
    m_grbl->sendCommand("M3 S1000");
}

// Untested
void Head::vacuumOff()
{
    m_vacuum_state = VACUUM_STATE::OFF;
    m_grbl->sendCommand("$32=0");
    m_grbl->sendCommand("M5");
}
