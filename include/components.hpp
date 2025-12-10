#ifndef PARSE_H
#define PARSE_H

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

struct coords_t {
    float x;
    float y;
    float z;
};

using namespace std;

class Components {

    private:
        map<tuple<string, string>, vector<component_t>> m_notInLookup;
        map<tuple<string, cuttape_t>, vector<component_t>> m_placement_map;

        map<tuple<string, cuttape_t>, vector<component_t>>::iterator m_cuttape_it;
        vector<component_t>::iterator m_component_it;

    public:

        void parseCSV(const char* csvFile);

        void addComponentLookUp(component_t component);
        void fillLostCuttapes();

        void printComponents();

        cuttape_t getCurrentCutTape();
        component_t getCurrentComponent();
        map<tuple<string, cuttape_t>, vector<component_t>>* getPlacementMap();

        vector<component_t>::iterator getComponent_it();
        map<tuple<string, cuttape_t>, vector<component_t>>::iterator getCutTape_it();

        void incrementCurrentComponent();
        void incrementCurrentCutTape();

};

#endif /* PARSE_H */