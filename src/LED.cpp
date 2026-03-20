#include "LED.hpp"

// Setters
void LED::setBrightness(float brightness)
{
    m_brightness = brightness;
    // m_grbl->sendCommand();
}

void LED::setOn()
{
    m_state = LED_STATE::ON;
    // m_grbl->sendCommand();
}

void LED::setOff()
{
    m_state = LED_STATE::OFF;
    // m_grbl->sendCommand();
}

void LED::setColor(LED_COLOR color)
{
    m_color = color;
    // m_grbl->sendCommand();
}

// Getters
LED_STATE LED::getState()
{
    return m_state;
}

float LED::getBrightness()
{
    return m_brightness;
}

LED_COLOR LED::getColor()
{
    return m_color;
}
