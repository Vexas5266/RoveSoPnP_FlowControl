
#include "gantry.hpp"

// Untested
void Gantry::setGlobalPosition(float x, float y)
{
    m_global_position.x = x;
    m_global_position.y = y;

    m_grbl->sendMotion("G90 G0 X" + std::to_string(m_global_position.x)
                           + " Y" + std::to_string(m_global_position.y));
}

// Untested
void Gantry::setHeadHeight(float z)
{
    m_global_position.z = z;
    m_grbl->sendMotion("G90 G0 Z" + std::to_string(m_global_position.z));
}

// Untested
void Gantry::home()
{
    m_global_position.x = 0;
    m_global_position.y = 0;
    m_global_position.z = 0; //Or change to top position

    m_grbl->sendMotion("$H");
}