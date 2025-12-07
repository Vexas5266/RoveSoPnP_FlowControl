#ifndef PARSE_H
#define PARSE_H

#include <string>
#include <sstream>
#include <fstream>

using namespace std;

string parseItemString(stringstream &s);
float parseItemFloat(stringstream &s);
int parseItemInt(stringstream &s);

#endif /* PARSE_H */