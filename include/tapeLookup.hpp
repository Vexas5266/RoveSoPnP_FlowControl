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

    bool operator<(const cuttape_t& other) const {
        return pitch < other.pitch;
    }
};



const int orientations_a[CNT_O] = {
    0,      /* NA */
    0,      /* C1 */
    -90,    /* C2 */
    180,    /* C3 */
    90,     /* C4 */
    -90,    /* M1 */
};

extern map<string, cuttape_t> cut_tape_map;

#endif