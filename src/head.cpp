
#include "head.hpp"

// Untested
void Head::increment(float degrees)
{
    m_grbl->sendCommand("G91 G0 A" + std::to_string(degrees));
}

// Untested
void Head::vacuumOn(int suckage)
{
    m_vacuum_state = VACUUM_STATE::ON;
    m_grbl->sendCommand("$32=0");
    m_grbl->sendCommand("M4 S" + std::to_string(suckage));
}

// Untested
void Head::vacuumOff()
{
    m_vacuum_state = VACUUM_STATE::OFF;
    m_grbl->sendCommand("$32=0");
    m_grbl->sendCommand("M5");
}
