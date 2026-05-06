#ifndef TAPELOOKUP_H
#define TAPELOOKUP_H

#include <map>
#include <string>

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
    int ID = -1;
    float pitch = -1;
    float width = -1;
    orientation_t orient = NA_O;

    bool operator<(const cuttape_t& other) const { return ID < other.ID; }
};

const int orientations_a[CNT_O] = {
    0,      /* NA */
    0,      /* C1 */
    -90,    /* C2 */
    180,    /* C3 */
    90,     /* C4 */
    -90,    /* M1 */
};

extern std::map<std::string, cuttape_t> cut_tape_map;

#endif