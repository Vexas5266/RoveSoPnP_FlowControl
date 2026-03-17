#ifndef LED_H
#define LED_H

#include "grbl.hpp"

enum class LED_STATE {
    ON,
    OFF,
    COUNT
};

enum class LED_COLOR {
    RED,
    ORANGE,
    YELLOW,
    GREEN,
    BLUE,
    PURPLE,
    WHITE,
    COUNT
};

class LED {

    private:
        LED_STATE m_state = LED_STATE::OFF;
        float m_brightness = 0; /* 0 to 1 */
        LED_COLOR m_color = LED_COLOR::WHITE;

        GRBL* m_grbl;

    public:
        LED(GRBL* grbl) { m_grbl = grbl; }
        ~LED() { m_grbl = NULL; } 
        
        // Setters
        void setBrightness(float brightness);
        void setOn();
        void setOff();
        void setColor(LED_COLOR color);

        // Getters
        LED_STATE getState();
        float getBrightness();
        LED_COLOR getColor();
};

#endif /* LED_H */