#ifndef GANTRY_H
#define GANTRY_H

#include "grbl.hpp"
#include <memory>

struct gantry_coords_t {
    float x;
    float y;
    float z;
};

class Gantry {
    private:
        std::shared_ptr<GRBL> m_grbl;

        gantry_coords_t m_global_position;

    public:
        Gantry(std::shared_ptr<GRBL> grbl) { m_grbl = grbl; }
        ~Gantry() { m_grbl = NULL; }

        void setGlobalPosition(float x, float y);
        void setHeadHeight(float z);
        void home();

        gantry_coords_t getGlobalPosition() { return m_global_position; }

        const gantry_coords_t offsetCamToHead = {0}; //TODO: Measure
};

#endif /* GANTRY_H */