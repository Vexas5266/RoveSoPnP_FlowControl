
#include "board.hpp"
#include "math.h"

Components::Components(const char* csvFile)
{
    std::ifstream file(csvFile);
    parseCSV(file);

    fillLostCuttapes();

    printComponents();

    m_cuttape_it   = m_placement_map.begin();
    m_component_it = m_cuttape_it->second.begin();
}

static void round(board_coords_t* coord) 
{ 
    coord->x = std::round(coord->x * 1000.0f) / 1000.0f;
    coord->y = std::round(coord->y * 1000.0f) / 1000.0f;
    coord->r = std::round(coord->r * 1000.0f) / 1000.0f;
}

static std::string parseItemString(std::stringstream& s)
{
    std::string data;
    getline(s, data, ',');
    data = data.substr(1, data.length() - 2);
    // cout << data << endl;
    return data;
}

static float parseItemFloat(std::stringstream& s)
{
    std::string data;
    getline(s, data, ',');
    // cout << data << endl;
    return stof(data);
}

static int parseItemInt(std::stringstream& s)
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
    while (getline(filestream, line, '\n'))
    {
        std::stringstream ss(line);
        component_t component = {
            parseItemString(ss),    // ref
            parseItemString(ss),    // value
            parseItemString(ss),    // package
            parseItemFloat(ss),     // posX
            parseItemFloat(ss),     // posY
            parseItemFloat(ss),     // rotation
            parseItemString(ss),    // side
        };

        Components::addComponentLookUp(component);
    }
}

void Components::addComponentLookUp(component_t component)
{
    if (component.ref == "REF**")
    {
        m_fiducials.push_back({component.posX, component.posY, component.rotation});
        return;
    }

    cuttape_t cuttape;

    std::map<std::string, cuttape_t>::iterator look_it = cut_tape_map.begin();
    while (look_it != cut_tape_map.end())
    {
        if (component.package.find(look_it->first) != std::string::npos)
            break;

        look_it++;
    }

    if (look_it == cut_tape_map.end())
    {
        // Not found in lookup
        m_notInLookup[{component.value, component.package}].push_back(component);
    }
    else
    {
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
        // Ask for cut tape info from user
        cuttape_t user_cuttape                                = {0, -1, -2, NA_O};
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
        status         = CHANGE_CUTTAPE;
    }

    if (m_cuttape_it == m_placement_map.end())
        status = FINAL_CUTTAPE;

    m_placed_components++;

    return status;
}

void Components::printComponents()
{
    int count                                                                             = 0;
    std::map<std::tuple<std::string, cuttape_t>, std::vector<component_t>>::iterator u_it = m_placement_map.begin();
    std::vector<component_t>::iterator c_it                                               = u_it->second.begin();
    std::vector<board_coords_t>::iterator f_it                      = m_fiducials.begin();

    while (u_it != m_placement_map.end())
    {
        std::cout << "Ref:" << c_it->ref << " Val:" << c_it->value << " CuttapeID:" << std::get<1>(u_it->first).ID << " Pkg:" << c_it->package << std::endl;

        c_it++;
        count++;

        // Handle iterators
        if (c_it == u_it->second.end())
        {
            u_it++;
            c_it = u_it->second.begin();
        }
    }

    while (f_it != m_fiducials.end())
    {
        std::cout << "Fiducials: " << " X:" << f_it->x << " Y:" << f_it->y << " R:" << f_it->r << std::endl;
        f_it++;
        count++;
    }

    std::cout << "Size: " << count << std::endl;
}

void Components::calculateBoardOffset(std::vector<board_coords_t> global_fiducials)
{
    int N                    = m_fiducials.size();
    board_coords_t transform = {0};

    if (N == 0)
        return;
    else if (N == 1)
    {
        transform.x = global_fiducials[0].x - m_fiducials[0].x;
        transform.y = global_fiducials[0].y - m_fiducials[0].y;
        transform.r = 0;
    }
    else if (N == 2)
    {
        double theta_P = atan2f(m_fiducials[1].y - m_fiducials[0].y,
                                m_fiducials[1].x - m_fiducials[0].x);
        double theta_Q = atan2f(global_fiducials[1].y - global_fiducials[0].y,
                                global_fiducials[1].x - global_fiducials[0].x);
        transform.r   = theta_Q - theta_P;

        transform.x   = global_fiducials[0].x - ((m_fiducials[0].x * cosf(transform.r)) 
                                              - (m_fiducials[0].y * sinf(transform.r)));
        transform.y = global_fiducials[0].y   - ((m_fiducials[0].x * sinf(transform.r))
                                              + (m_fiducials[0].y * cosf(transform.r)));
    }
    else if (N > 2)
    {
        double Px_bar = 0;
        double Py_bar = 0;
        double Qx_bar = 0;
        double Qy_bar = 0;
        for (int i = 0; i < N; i++)
        {
            Px_bar += m_fiducials[i].x;
            Py_bar += m_fiducials[i].y;
            Qx_bar += global_fiducials[i].x;
            Qy_bar += global_fiducials[i].y;
        }
        board_coords_t centroid_P = {Px_bar * ((double) 1.0 / N), Py_bar * ((double) 1.0 / N), 0};
        board_coords_t centroid_Q = {Qx_bar * ((double) 1.0 / N), Qy_bar * ((double) 1.0 / N), 0};

        double sinR = 0; 
        double cosR = 0;
        for (int i = 0; i < N; i++)
        {
            board_coords_t p, q;
            p.x = m_fiducials[i].x - centroid_P.x;
            p.y = m_fiducials[i].y - centroid_P.y;
            q.x = global_fiducials[i].x - centroid_Q.x;
            q.y = global_fiducials[i].y - centroid_Q.y;

            sinR += (p.x * q.y) - (p.y * q.x);
            cosR += (p.x * q.x) + (p.y * q.y);
        }
        transform.r = atan2(sinR, cosR);

        double c = cosf(transform.r);
        double s = sinf(transform.r);

        transform.x = centroid_Q.x - ((centroid_P.x * c) - (centroid_P.y * s));
        transform.y = centroid_Q.y - ((centroid_P.x * s) + (centroid_P.y * c));
    }
    else
        return;

    round(&transform);
    m_board_offset = transform;
}

void Components::transformToPCBCoords(board_coords_t* global_coords)
{
    board_coords_t pcb_coords;

    double c = cosf(-m_board_offset.r);
    double s = sinf(-m_board_offset.r);

    // Apply rotation
    pcb_coords.x = (global_coords->x * c) - (global_coords->y * s);
    pcb_coords.y = (global_coords->x * s) + (global_coords->y * c);

    // Apply translation
    pcb_coords.x = global_coords->x - m_board_offset.x;
    pcb_coords.y = global_coords->y - m_board_offset.y;

    pcb_coords.r = global_coords->r - RAD2DEG(m_board_offset.r);

    round(&pcb_coords);
    *global_coords = pcb_coords;
}

void Components::transformToGlobalCoords(board_coords_t* PCB_coords)
{
    board_coords_t global_coords;

    double c = cosf(m_board_offset.r);
    double s = sinf(m_board_offset.r);

    // Apply rotation
    global_coords.x = (PCB_coords->x * c) - (PCB_coords->y * s);
    global_coords.y = (PCB_coords->x * s) + (PCB_coords->y * c);

    // Apply translation
    global_coords.x = PCB_coords->x + m_board_offset.x;
    global_coords.y = PCB_coords->y + m_board_offset.y;

    global_coords.r = PCB_coords->r + RAD2DEG(m_board_offset.r);

    round(&global_coords);
    *PCB_coords  = global_coords;
}

void Components::printCoords(board_coords_t coords)
{

    std::cout << "Offset   x:" << m_board_offset.x << "   y:" << m_board_offset.y << "   r:" << RAD2DEG(m_board_offset.r) << std::endl;
    std::cout << "Test     x:" << coords.x << "   y:" << coords.y << "   r:" << coords.r << std::endl;
}
