#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <iostream>

#include "tapeLookup.hpp"

struct component_t {
    string ref;
    string value;
    string package;
    float posX;
    float posY;
    float rotation;
    string side;
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

    public:
        Components(const char* csvFile);

        void addComponentLookUp(component_t component);
        void fillLostCuttapes();
        void parseCSV(ifstream& filestream);

        void printComponents();

        cuttape_t getCurrentCutTape();
        component_t getCurrentComponent();
        map<tuple<string, cuttape_t>, vector<component_t>> getPlacementMap();

        // After incrementing, returns if it had to move to a new cuttape, 
        // is in the same cuttape, or is at the end of the map
        components_status_t incrementCurrentComponent();

};

#endif /* COMPONENTS_H */