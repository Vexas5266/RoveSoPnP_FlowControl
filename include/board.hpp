#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <iostream>

#include "tapeLookup.hpp"

#define P_EXP 0 //Board Fiducial
#define Q_MEAS 1 //Manual Fiducial

struct component_t {
    std::string ref;
    std::string value;
    std::string package;
    float posX;
    float posY;
    float rotation;
    std::string side;
};

struct board_coords_t {
    double x;
    double y;
    double r;
};

#define DEG2RAD(x) (x * M_PI) / 180.0
#define RAD2DEG(x) (180.0 * x) / M_PI

enum components_status_t {
    SAME_CUTTAPE,
    CHANGE_CUTTAPE,
    FINAL_CUTTAPE
};

class Components {

    private:
        std::map<std::tuple<std::string, std::string>, std::vector<component_t>> m_notInLookup;
        std::map<std::tuple<std::string, cuttape_t>, std::vector<component_t>> m_placement_map; // <[Package, Cuttape[P W O]], <Vector of all comp with that package and cuttape>>

        std::map<std::tuple<std::string, cuttape_t>, std::vector<component_t>>::iterator m_cuttape_it;
        std::vector<component_t>::iterator m_component_it;

        std::vector<board_coords_t> m_fiducials;
        board_coords_t m_board_offset = {0, 0, 0}; // {mm, mm, radians}

        int m_placed_components = 0;

    public:
        Components(const char* csvFile);

        void addComponentLookUp(component_t component);
        void fillLostCuttapes();
        void parseCSV(std::ifstream& filestream);

        void printComponents();

        cuttape_t getCurrentCutTape() { return std::get<1>(m_cuttape_it->first); }
        component_t getCurrentComponent() { return *m_component_it; }
        std::map<std::tuple<std::string, cuttape_t>, std::vector<component_t>> getPlacementMap() { return m_placement_map; }
        int getPlacedComponents() { return m_placed_components; }

        std::vector<board_coords_t> getBoardFiducials() { return m_fiducials; }
        void calculateBoardOffset(std::vector<board_coords_t> global_fiducials);
        void transformToPCBCoords(board_coords_t* global_coords);
        void transformToGlobalCoords(board_coords_t* PCB_coords);
        
        void printCoords(board_coords_t coords);

        // After incrementing, returns if it had to move to a new cuttape, 
        // is in the same cuttape, or is at the end of the map
        components_status_t incrementCurrentComponent();

};

#endif /* BOARD_H */