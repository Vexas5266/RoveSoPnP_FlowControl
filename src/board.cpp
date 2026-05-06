
#include "board.hpp"
#include "math.h"

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

void Components::parseCSV(const char* csvFile)
{
    m_csv_file = csvFile;
    std::ifstream file(csvFile);

    m_placement_map.clear();
    m_lost_cuttape_map.clear();
    m_fiducials.clear();
    m_placed_components = 0;
    m_board_offset      = {0, 0, 0};

    if (!file.is_open())
    {
        std::cout << "Unable to open CSV file: " << csvFile << std::endl;
        return;
    }

    std::string line;
    getline(file, line, '\n');
    while (getline(file, line, '\n'))
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

    m_cuttape_it = m_placement_map.begin();
    if (m_cuttape_it != m_placement_map.end())
        m_component_it = m_cuttape_it->second.begin();

    printComponents();
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
        m_lost_cuttape_map[{component.value, component.package}].push_back(component);
    }
    else
    {
        cuttape = {look_it->second.ID, look_it->second.pitch, look_it->second.width, look_it->second.orient};
        m_placement_map[{component.value, component.package, cuttape}].push_back(component);
    }
}

void Components::updateCuttapeInPlacementMap(std::string value, std::string package, cuttape_t old_cuttape, cuttape_t user_cuttape)
{
    placement_map_t::iterator cuttape_it = m_placement_map.find({value, package, old_cuttape});
    std::vector<component_t> components  = cuttape_it->second;
    m_placement_map.erase(cuttape_it);
    m_placement_map[{value, package, user_cuttape}] = components;
}

void Components::addLostCuttapeToPlacementMap(std::string value, std::string package, cuttape_t user_cuttape)
{
    lost_cuttape_map_t::iterator it = m_lost_cuttape_map.find({value, package});
    if (it == m_lost_cuttape_map.end())
        return;    // Didn't find lost cuttape with that value or package

    // If user's cuttape is valid
    if (user_cuttape.ID > -1)
        m_placement_map[{value, package, user_cuttape}] = it->second;

    m_lost_cuttape_map.erase(it);
}

void Components::removeFromPlacementMapToLostMap(std::string value, std::string package, cuttape_t user_cuttape)
{
    placement_map_t::iterator it = m_placement_map.find({value, package, user_cuttape});
    if (it == m_placement_map.end())
        return;    // Didn't find lost cuttape with that value or package

    m_lost_cuttape_map[{value, package}] = it->second;
    m_placement_map.erase(it);
}

Components::components_status_t Components::incrementCurrentComponent()
{
    if (m_placement_map.empty() || m_cuttape_it == m_placement_map.end())
        return FINAL_CUTTAPE;

    components_status_t status = SAME_CUTTAPE;

    m_component_it++;
    if (m_component_it == m_cuttape_it->second.end())
    {
        m_cuttape_it++;
        status = CHANGE_CUTTAPE;

        if (m_cuttape_it == m_placement_map.end())
        {
            m_placed_components++;
            return FINAL_CUTTAPE;
        }

        m_component_it = m_cuttape_it->second.begin();
    }

    m_placed_components++;

    return status;
}

void Components::printComponents()
{
    int count                                  = 0;
    placement_map_t::iterator u_it             = m_placement_map.begin();
    std::vector<board_coords_t>::iterator f_it = m_fiducials.begin();

    while (u_it != m_placement_map.end())
    {
        std::cout << "V:" << get<VAL>(u_it->first) << " P:" << get<PKG>(u_it->first) << " CTID:" << get<CTP>(u_it->first).ID << std::endl;
        std::vector<component_t>::iterator c_it = u_it->second.begin();
        while (c_it != u_it->second.end())
        {
            std::cout << "      R:" << c_it->ref << std::endl;
            c_it++;
            count++;
        }
        u_it++;
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
        double theta_P = atan2f(m_fiducials[1].y - m_fiducials[0].y, m_fiducials[1].x - m_fiducials[0].x);
        double theta_Q = atan2f(global_fiducials[1].y - global_fiducials[0].y, global_fiducials[1].x - global_fiducials[0].x);
        transform.r    = theta_Q - theta_P;

        transform.x    = global_fiducials[0].x - ((m_fiducials[0].x * cosf(transform.r)) - (m_fiducials[0].y * sinf(transform.r)));
        transform.y    = global_fiducials[0].y - ((m_fiducials[0].x * sinf(transform.r)) + (m_fiducials[0].y * cosf(transform.r)));
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

        double sinR               = 0;
        double cosR               = 0;
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

        double c    = cosf(transform.r);
        double s    = sinf(transform.r);

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
    pcb_coords.x -= m_board_offset.x;
    pcb_coords.y -= m_board_offset.y;

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
    global_coords.x += m_board_offset.x;
    global_coords.y += m_board_offset.y;

    global_coords.r = PCB_coords->r + RAD2DEG(m_board_offset.r);

    round(&global_coords);
    *PCB_coords = global_coords;
}

void Components::printCoords(board_coords_t coords)
{
    std::cout << "Offset   x:" << m_board_offset.x << "   y:" << m_board_offset.y << "   r:" << RAD2DEG(m_board_offset.r) << std::endl;
    std::cout << "Test     x:" << coords.x << "   y:" << coords.y << "   r:" << coords.r << std::endl;
}
