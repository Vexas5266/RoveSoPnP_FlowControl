#ifndef TAPELOOKUP_H
#define TAPELOOKUP_H

#include <map>
#include <string>

using namespace std;

enum orientation_t {
    NA_O,
    C1_O,
    C2_O,
    C3_O,
    C4_O,
    M1_O,
    CNT_O
};

struct cuttape_t {
    float pitch;
    float width;
    orientation_t orient;
};

const int orientations_a[CNT_O] = {
    0,   /* NA */
    90,  /* C1 */
    0,   /* C2 */
    -90, /* C3 */
    180, /* C4 */
    0,   /* M1 */
};

const map<string, cuttape_t> cut_tape_map = {
    {"0603", {4.0, 6.0, C2_O}},
    {"0603", {4.0, 6.0, C2_O}},
};

#endif