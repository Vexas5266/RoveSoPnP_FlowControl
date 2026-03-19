#ifndef HEAD_H
#define HEAD_H

#include "grbl.hpp"
#include <memory>

enum class VACUUM_STATE {
    ON,
    OFF,
    COUNT
};

class Head {
    private:
        std::shared_ptr<GRBL> m_grbl;
        int m_angle;
        VACUUM_STATE m_vacuum_state = VACUUM_STATE::OFF;

    public:
        Head(std::shared_ptr<GRBL> grbl) { m_grbl = grbl; }
        ~Head() { m_grbl = NULL; } 

        void increment(int degrees);
        void vacuumOn();
        void vacuumOff();

        VACUUM_STATE getVacuumState() { return m_vacuum_state; }
        int getAngle() { return m_angle; }
};

#endif /* HEAD_H */