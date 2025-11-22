
#include "parse.hpp"

string parseItemString(stringstream &s)
{
    string data;
    getline(s, data, ',');
    data = data.substr(1, data.length() - 2);
    // cout << data << endl;
    return data;
}

float parseItemFloat(stringstream &s)
{
    string data;
    getline(s, data, ',');
    // cout << data << endl;
    return stof(data);
}

int parseItemInt(stringstream &s)
{
    string data;
    getline(s, data, ',');
    // cout << data << endl;
    return stoi(data);
}