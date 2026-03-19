
#include "board.hpp"
#include "math.h"

Components::Components(const char* csvFile)
{
    std::ifstream file(csvFile);
    parseCSV(file);

    fillLostCuttapes();

    printComponents();

    m_cuttape_it = m_placement_map.begin();
    m_component_it = m_cuttape_it->second.begin();
}

static std::string parseItemString(std::stringstream &s)
{
    std::string data;
    getline(s, data, ',');
    data = data.substr(1, data.length() - 2);
    // cout << data << endl;
    return data;
}

static float parseItemFloat(std::stringstream &s)
{
    std::string data;
    getline(s, data, ',');
    // cout << data << endl;
    return stof(data);
}

static int parseItemInt(std::stringstream &s)
{
    std::string data;
    getline(s, data, ',');
    // cout << data << endl;
    return stoi(data);
}

void Components::parseCSV(std::ifstream& filestream)
{
    std::string line;
    getline(filestream, line, '\n');
    while (getline(filestream, line, '\n')) {
        std::stringstream ss(line);
        component_t component = {
            parseItemString(ss), //ref
            parseItemString(ss), //value
            parseItemString(ss), //package
            parseItemFloat(ss), //posX
            parseItemFloat(ss), //posY
            parseItemFloat(ss), //rotation
            parseItemString(ss), //side
        };

        Components::addComponentLookUp(component);
    }
}

void Components::addComponentLookUp(component_t component)
{
    if (component.ref == "REF**")
    {
        m_fiducials.push_back({component, {"MAN**", "ManFiducial", "ManFiducial", 0, 0, 0, "top"}});
        return;
    }

    cuttape_t cuttape;

    std::map<std::string, cuttape_t>::iterator look_it = cut_tape_map.begin();
    while (look_it != cut_tape_map.end())
    {
        if (component.package.find(look_it->first) != std::string::npos) break;

        look_it++;
    }

    if (look_it == cut_tape_map.end()) 
    {
        // Not found in lookup
        m_notInLookup[{component.value, component.package}].push_back(component);
    } else {
        // cout << "Found cut tape!!  " << component.ref << endl;
        cuttape = {look_it->second.ID, look_it->second.pitch, look_it->second.width, look_it->second.orient};
        m_placement_map[{component.value, cuttape}].push_back(component);
    }

}

void Components::fillLostCuttapes()
{
    std::map<std::tuple<std::string, std::string>, std::vector<component_t>>::iterator lost_cuttape_it = m_notInLookup.begin();

    while (lost_cuttape_it != m_notInLookup.end())
    {
        //Ask for cut tape info from user
        cuttape_t user_cuttape = {0, -1, -2, NA_O};
        std::vector<component_t>::iterator individual_comp_it = lost_cuttape_it->second.begin();
        while (individual_comp_it != lost_cuttape_it->second.end())
        {
            m_placement_map[{individual_comp_it->value, user_cuttape}].push_back(*individual_comp_it);
            individual_comp_it++;
        }

        lost_cuttape_it++;
    }
}

components_status_t Components::incrementCurrentComponent()
{
    components_status_t status = SAME_CUTTAPE;

    m_component_it++;
    if (m_component_it == m_cuttape_it->second.end())
    {
        m_cuttape_it++;
        m_component_it = m_cuttape_it->second.begin();
        status = CHANGE_CUTTAPE;
    }

    if (m_cuttape_it == m_placement_map.end()) status = FINAL_CUTTAPE;

    m_placed_components++;

    return status;
}

void Components::printComponents()
{
    int count = 0;
    std::map<std::tuple<std::string, cuttape_t>, std::vector<component_t>>::iterator u_it = m_placement_map.begin();
    std::vector<component_t>::iterator c_it = u_it->second.begin();
    std::vector<std::tuple<component_t, component_t>>::iterator f_it = m_fiducials.begin();

    while (u_it != m_placement_map.end())
    {
        std::cout << "Ref:" << c_it->ref << " Val:" << c_it->value << " CuttapeID:" << std::get<1>(u_it->first).ID << " Pkg:" << c_it->package << std::endl;

        c_it++;
        count++;
        
        //Handle iterators
        if (c_it == u_it->second.end()) 
        {
            u_it++;
            c_it = u_it->second.begin();
        }
    }

    while (f_it != m_fiducials.end())
    {
        std::cout << "Fiducials: " << (std::get<0>(*f_it)).value << " X:" << (std::get<0>(*f_it)).posX << " Y:" << (std::get<0>(*f_it)).posY << std::endl;
        std::cout << "Manual Fiducial: " << (std::get<1>(*f_it)).value << " X:" << (std::get<1>(*f_it)).posX << " Y:" << (std::get<1>(*f_it)).posY << std::endl;
        f_it++;
        count++;
    }

    std::cout << "Size: " << count << std::endl;
}

void Components::setManualFidcucials(int idx, board_coords_t manual_coords)
{
    std::get<1>(m_fiducials[idx]).posX = manual_coords.x;
    std::get<1>(m_fiducials[idx]).posY = manual_coords.y;
}

void Components::calculateBoardOffset()
{
    int N = m_fiducials.size();
    board_coords_t transform = {0, 0, 0, 0};

    if (N == 0) return;
    else if (N == 1)
    {
        transform.x = std::get<Q_MEAS>(m_fiducials[0]).posX - std::get<P_EXP>(m_fiducials[0]).posX;
        transform.y = std::get<Q_MEAS>(m_fiducials[0]).posY - std::get<P_EXP>(m_fiducials[0]).posY;
        transform.r = 0;
    } 
    else if (N == 2)
    {
        float theta_P = atan2f(std::get<P_EXP> (m_fiducials[1]).posY - std::get<P_EXP> (m_fiducials[0]).posY,
                               std::get<P_EXP> (m_fiducials[1]).posX - std::get<P_EXP> (m_fiducials[0]).posX);
        float theta_Q = atan2f(std::get<Q_MEAS>(m_fiducials[1]).posY - std::get<Q_MEAS>(m_fiducials[0]).posY,
                               std::get<Q_MEAS>(m_fiducials[1]).posX - std::get<Q_MEAS>(m_fiducials[0]).posX);
        transform.r = theta_Q - theta_P;

        transform.x = std::get<Q_MEAS>(m_fiducials[0]).posX - ( (std::get<P_EXP>(m_fiducials[0]).posX * cosf(transform.r)) - 
                                                           (std::get<P_EXP>(m_fiducials[0]).posY * sinf(transform.r)) );
        transform.y = std::get<Q_MEAS>(m_fiducials[0]).posY - ( (std::get<P_EXP>(m_fiducials[0]).posX * sinf(transform.r)) + 
                                                           (std::get<P_EXP>(m_fiducials[0]).posY * cosf(transform.r)) );
    }
    else if (N > 2)
    {
        float Px_bar = 0; 
        float Py_bar = 0; 
        float Qx_bar = 0; 
        float Qy_bar = 0;
        for (int i = 0; i < N; i++)
        {
            Px_bar += std::get<P_EXP>(m_fiducials[i]).posX;
            Py_bar += std::get<P_EXP>(m_fiducials[i]).posY;
            Qx_bar += std::get<Q_MEAS>(m_fiducials[i]).posX;
            Qy_bar += std::get<Q_MEAS>(m_fiducials[i]).posY;
        }
        board_coords_t centroid_P = { Px_bar * ((float)1.0/N), Py_bar * ((float)1.0/N), 0, 0};
        board_coords_t centroid_Q = { Qx_bar * ((float)1.0/N), Qy_bar * ((float)1.0/N), 0, 0};

        double sinR, cosR = 0;
        for (int i = 0; i < N; i++)
        {
            board_coords_t p, q;
            p.x = std::get<P_EXP>(m_fiducials[i]).posX - centroid_P.x;
            p.y = std::get<P_EXP>(m_fiducials[i]).posY - centroid_P.y;
            q.x = std::get<Q_MEAS>(m_fiducials[i]).posX - centroid_Q.x;
            q.y = std::get<Q_MEAS>(m_fiducials[i]).posY - centroid_Q.y;

            sinR += (p.x * q.y) - (p.y * q.x);
            cosR += (p.x * q.x) + (p.y * q.y);
        }
        transform.r = atan2(sinR, cosR);

        transform.x = Qx_bar - ( (Px_bar*cosf(transform.r)) - Py_bar*sinf(transform.r) );
        transform.y = Qy_bar - ( (Px_bar*sinf(transform.r)) + Py_bar*cosf(transform.r) );
    }
    else return;

    m_board_offset = transform;
}

void Components::transformToPCBCoords(board_coords_t* global_coords)
{
    board_coords_t new_coords;

    new_coords.x = (global_coords->x * cosf(m_board_offset.r)) - 
                   (global_coords->y * sinf(m_board_offset.r)) + m_board_offset.x;
    new_coords.y = (global_coords->x * sinf(m_board_offset.r)) + 
                   (global_coords->y * cosf(m_board_offset.r)) + m_board_offset.y;
    new_coords.r = global_coords->r + m_board_offset.r;
    std::cout << " offsetR:" << m_board_offset.r << std::endl;
    std::cout << " globalR:" << global_coords->r << std::endl;
    std::cout << " globalx:" << global_coords->x << std::endl;
    std::cout << " globaly:" << global_coords->y << std::endl;
    std::cout << " newR:" << new_coords.r << std::endl;

    *global_coords = new_coords;
}

void Components::transformToGlobalCoords(board_coords_t* PCB_coords)
{
    board_coords_t new_coords;

    new_coords.x = (PCB_coords->x - m_board_offset.x);
    new_coords.y = (PCB_coords->y - m_board_offset.y);

    new_coords.x = (new_coords.x * cosf(m_board_offset.r)) + 
                   (new_coords.y * sinf(m_board_offset.r));
    new_coords.y = (-new_coords.x * sinf(m_board_offset.r)) + 
                   (new_coords.y * cosf(m_board_offset.r));

    new_coords.r = PCB_coords->r - m_board_offset.r;

    *PCB_coords = new_coords;
}