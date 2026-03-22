#ifndef GANTRY_H
#define GANTRY_H

#include "grbl.hpp"
#include <memory>

class Gantry
{
    private:
        std::shared_ptr<GRBL> m_grbl;

        grbl_position_t m_global_position;

    public:
        Gantry(std::shared_ptr<GRBL> grbl) { m_grbl = grbl; }

        ~Gantry() { m_grbl = NULL; }

        void setGlobalPosition(float x, float y);
        void setHeadHeight(float z);
        void home();

        grbl_position_t getGlobalPosition() { return m_global_position; }

        const grbl_position_t offsetCamToHead = {0, 0, 0};    // TODO: Measure
};

#endif /* GANTRY_H */
