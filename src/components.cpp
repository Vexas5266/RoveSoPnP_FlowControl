
#include "components.hpp"

static string parseItemString(stringstream &s);
static float parseItemFloat(stringstream &s);
static int parseItemInt(stringstream &s);

static string parseItemString(stringstream &s)
{
    string data;
    getline(s, data, ',');
    data = data.substr(1, data.length() - 2);
    // cout << data << endl;
    return data;
}

static float parseItemFloat(stringstream &s)
{
    string data;
    getline(s, data, ',');
    // cout << data << endl;
    return stof(data);
}

static int parseItemInt(stringstream &s)
{
    string data;
    getline(s, data, ',');
    // cout << data << endl;
    return stoi(data);
}

void Components::parseCSV(const char* csvFile)
{
    ifstream file(csvFile);
    string line;
    getline(file, line, '\n');
    while (getline(file, line, '\n')) {
        stringstream ss(line);
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
    m_cuttape_it = m_placement_map.begin();
    m_component_it = m_cuttape_it->second.begin();
}

void Components::addComponentLookUp(component_t component)
{
    cuttape_t cuttape;

    map<string, cuttape_t>::iterator look_it = cut_tape_map.begin();
    while (look_it != cut_tape_map.end())
    {
        if (component.package.find(look_it->first) != string::npos) break;

        look_it++;
    }

    if (look_it == cut_tape_map.end()) 
    {
        // Not found in lookup
        m_notInLookup[{component.value, component.package}].push_back(component);
    } else {
        // cout << "Found cut tape!!  " << component.ref << endl;
        cuttape = {look_it->second.pitch, look_it->second.width, look_it->second.orient};
        m_placement_map[{component.value, cuttape}].push_back(component);
    }

}

void Components::fillLostCuttapes()
{
    map<tuple<string, string>, vector<component_t>>::iterator lost_cuttape_it = m_notInLookup.begin();

    while (lost_cuttape_it != m_notInLookup.end())
    {
        //Ask for cut tape info from user
        cuttape_t user_cuttape = {-1, -2, NA_O};
        vector<component_t>::iterator individual_comp_it = lost_cuttape_it->second.begin();
        while (individual_comp_it != lost_cuttape_it->second.end())
        {
            m_placement_map[{individual_comp_it->value, user_cuttape}].push_back(*individual_comp_it);
            individual_comp_it++;
        }

        lost_cuttape_it++;
    }
}

component_t Components::getCurrentComponent()
{
    return *m_component_it;
}

vector<component_t>::iterator Components::getComponent_it()
{
    return m_component_it;
}

void Components::incrementCurrentComponent()
{
    m_component_it++;
}

void Components::incrementCurrentCutTape()
{
    m_cuttape_it++;
    m_component_it = m_cuttape_it->second.begin();
}

map<tuple<string, cuttape_t>, vector<component_t>>::iterator Components::getCutTape_it()
{
    return m_cuttape_it;
}

cuttape_t Components::getCurrentCutTape()
{
    return get<1>(m_cuttape_it->first);
}

map<tuple<string, cuttape_t>, vector<component_t>>* Components::getPlacementMap()
{
    return &m_placement_map;
}

void Components::printComponents()
{
    int count = 0;
    map<tuple<string, cuttape_t>, vector<component_t>>::iterator u_it = m_placement_map.begin();
    vector<component_t>::iterator c_it = u_it->second.begin();
    while (u_it != m_placement_map.end())
    {
        cout << "Ref:" << c_it->ref << " Val:" << c_it->value << " Width:" << get<1>(u_it->first).width << " Pkg:" << c_it->package << endl;

        c_it++;
        count++;
        
        //Handle iterators
        if (c_it == u_it->second.end()) 
        {
            u_it++;
            c_it = u_it->second.begin();
        }
    }

    cout << "Size: " << count << endl;
}