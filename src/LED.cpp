#include "LED.hpp"

std::tuple<uint8_t, uint8_t, uint8_t> LED::colorToRGB(LED_COLOR color)
{
    switch (color)
    {
        case LED_COLOR::RED: return {255, 0, 0};
        case LED_COLOR::ORANGE: return {255, 165, 0};
        case LED_COLOR::YELLOW: return {255, 255, 0};
        case LED_COLOR::GREEN: return {0, 255, 0};
        case LED_COLOR::BLUE: return {0, 0, 255};
        case LED_COLOR::PURPLE: return {128, 0, 128};
        case LED_COLOR::WHITE: return {255, 255, 255};
        default: return {255, 255, 255};    // Default to white if unknown
    }
}

// Setters
void LED::setBrightness(float brightness)
{
    m_brightness = brightness;
    // m_grbl->sendCommand();
}

void LED::setOn()
{
    m_state        = LED_STATE::ON;
    auto rgb_color = colorToRGB(m_color);
    std::cout << "LED Color RGB before brightness: " << std::get<0>(rgb_color) << " " << std::get<1>(rgb_color) << " " << std::get<2>(rgb_color);
    std::get<0>(rgb_color) = std::get<0>(rgb_color) * (float) m_brightness;
    std::get<1>(rgb_color) = std::get<1>(rgb_color) * (float) m_brightness;
    std::get<2>(rgb_color) = std::get<2>(rgb_color) * (float) m_brightness;
    std::string cmd        = "M69 P" + std::to_string(get<0>(rgb_color)) + " R" + std::to_string(get<1>(rgb_color)) + " S" + std::to_string(get<2>(rgb_color));
    m_grbl->sendCommand(cmd);
}

void LED::setOff()
{
    m_state = LED_STATE::OFF;
    m_grbl->sendCommand("M69 P0 R0 S0");    // Command to turn off the first led, need to update to support two leds
}

void LED::setColor(LED_COLOR color)
{
    m_color = color;
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
