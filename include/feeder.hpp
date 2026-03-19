#ifndef FEEDER_H
#define FEEDER_H

#include "grbl.hpp"
#include "board.hpp"
#include <memory>

class Feeder {
    private:
        std::shared_ptr<GRBL> m_grbl;

    public:
        Feeder(std::shared_ptr<GRBL> grbl) { m_grbl = grbl; }
        ~Feeder() { m_grbl = NULL; } 

        void increment(float length) { m_grbl->sendMotion("G91 G0 B" + std::to_string(length)); }

};

#endif /* FEEDER_H */