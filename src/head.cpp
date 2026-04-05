
#include "head.hpp"

// Untested
void Head::increment(int degrees)
{
    m_grbl->sendCommand("G91 G1 F500 A" + std::to_string(degrees));
}

// Untested
void Head::vacuumOn()
{
    m_vacuum_state = VACUUM_STATE::ON;
    m_grbl->sendCommand("$32=0");
    m_grbl->sendCommand("M4 S1000");
}

// Untested
void Head::vacuumOff()
{
    m_vacuum_state = VACUUM_STATE::OFF;
    m_grbl->sendCommand("$32=0");
    m_grbl->sendCommand("M5");
}
