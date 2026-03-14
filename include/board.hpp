#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <iostream>

#include "tapeLookup.hpp"

#define P_EXP 0
#define Q_MEAS 1

#define DEG2RAD(x) (x * M_PI) / 180.0
#define RAD2DEG(x) (180.0 * x) / M_PI

struct component_t {
    string ref;
    string value;
    string package;
    float posX;
    float posY;
    float rotation;
    string side;
};

struct coords_t {
    float x;
    float y;
    float z;
    float r;
};

enum components_status_t {
    SAME_CUTTAPE,
    CHANGE_CUTTAPE,
    FINAL_CUTTAPE
};

using namespace std;

class Components {

    private:
        map<tuple<string, string>, vector<component_t>> m_notInLookup;
        map<tuple<string, cuttape_t>, vector<component_t>> m_placement_map; // <[Package, Cuttape[P W O]], <Vector of all comp with that package and cuttape>>

        map<tuple<string, cuttape_t>, vector<component_t>>::iterator m_cuttape_it;
        vector<component_t>::iterator m_component_it;

        vector<tuple<component_t, component_t>> m_fiducials; // <Board, manual>
        coords_t m_board_offset = {0, 0, 0, 0};

        int m_placed_components = 0;

    public:
        Components(const char* csvFile);

        void addComponentLookUp(component_t component);
        void fillLostCuttapes();
        void parseCSV(ifstream& filestream);

        void printComponents();

        cuttape_t getCurrentCutTape() { return get<1>(m_cuttape_it->first); }
        component_t getCurrentComponent() { return *m_component_it; }
        map<tuple<string, cuttape_t>, vector<component_t>> getPlacementMap() { return m_placement_map; }
        int getPlacedComponents() { return m_placed_components; }

        vector<tuple<component_t, component_t>> getBoardFiducials() { return m_fiducials; }
        void setManualFidcucials(int idx, coords_t manual_coords);
        void calculateBoardOffset();
        coords_t getBoardOffset() { return m_board_offset; }
        void transformToPCBCoords(coords_t* global_coords);
        void transformToGlobalCoords(coords_t* PCB_coords);

        // After incrementing, returns if it had to move to a new cuttape, 
        // is in the same cuttape, or is at the end of the map
        components_status_t incrementCurrentComponent();

};

#endif /* COMPONENTS_H */