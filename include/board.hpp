#ifndef BOARD_H
#define BOARD_H

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "tapeLookup.hpp"

#define DEG2RAD(x) (x * M_PI) / 180.0
#define RAD2DEG(x) (180.0 * x) / M_PI

struct board_coords_t
{
        double x;
        double y;
        double r;
};

class Components
{
    public:
        enum components_status_t
        {
            SAME_CUTTAPE,
            CHANGE_CUTTAPE,
            FINAL_CUTTAPE
        };

        enum map_sort_t
        {
            VAL,
            PKG,
            CTP
        };

        struct component_t
        {
                std::string ref     = "";
                std::string value   = "";
                std::string package = "";
                float posX          = 0.0;
                float posY          = 0.0;
                float rotation      = 0.0;
                std::string side    = "";
        };

        // <[Value, Package, Cuttape[P W O]], <Vector of all comp with that package and cuttape>>
        typedef std::map<std::tuple<std::string, std::string, cuttape_t>, std::vector<component_t>> placement_map_t;
        typedef std::map<std::tuple<std::string, std::string>, std::vector<component_t>> lost_cuttape_map_t;

        ~Components() { std::cout << "Components destroyed" << std::endl; }

        void addComponentLookUp(component_t component);
        void updateCuttapeInPlacementMap(std::string value, std::string package, cuttape_t old_cuttape, cuttape_t user_cuttape);
        void addLostCuttapeToPlacementMap(std::string value, std::string package, cuttape_t user_cuttape);
        void removeFromPlacementMapToLostMap(std::string value, std::string package, cuttape_t user_cuttape);

        void parseCSV(const char* csvFile);

        void printComponents();

        cuttape_t getCurrentCutTape() { return std::get<CTP>(m_cuttape_it->first); }

        component_t getCurrentComponent() { return *m_component_it; }

        placement_map_t getPlacementMap() { return m_placement_map; }

        int getPlacedComponents() { return m_placed_components; }

        std::vector<board_coords_t> getBoardFiducials() { return m_fiducials; }

        lost_cuttape_map_t getLostCuttapes() { return m_lost_cuttape_map; }

        void calculateBoardOffset(std::vector<board_coords_t> global_fiducials);
        void transformToPCBCoords(board_coords_t* global_coords);
        void transformToGlobalCoords(board_coords_t* PCB_coords);

        void printCoords(board_coords_t coords);

        // After incrementing, returns if it had to move to a new cuttape,
        // is in the same cuttape, or is at the end of the map
        components_status_t incrementCurrentComponent();

    private:
        const char* m_csv_file = "";
        placement_map_t m_placement_map;
        lost_cuttape_map_t m_lost_cuttape_map;

        placement_map_t::iterator m_cuttape_it;
        std::vector<component_t>::iterator m_component_it;

        std::vector<board_coords_t> m_fiducials;
        board_coords_t m_board_offset = {0, 0, 0};    // {mm, mm, radians}

        int m_placed_components       = 0;
};

#endif /* BOARD_H */
