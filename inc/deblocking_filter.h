#ifndef DEBLOCKING_FILTER
#define DEBLOCKING_FILTER

#include <vector>
#include <array>
#include <cmath>

#include "macroblock.h"
#include "frame.h"
#include "qdct.h"

static const std::array<int, 52> alpha{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 4, 4, 5, 6,
                                        7, 8, 9, 10, 12, 13, 15, 17, 20, 22,
                                        25, 28, 32, 36, 40, 45, 50, 56, 63, 71,
                                        80, 90, 101, 113, 127, 144, 162, 182, 203, 226,
                                        255, 255}};

static const std::array<int, 52> beta{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 2, 2, 2, 3,
                                       3, 3, 3, 4, 4, 4, 6, 6, 7, 7,
                                       8, 8, 9, 9, 10, 10, 11, 11, 12, 12,
                                       13, 13, 14, 14, 15, 15, 16, 16, 17, 17,
                                       18, 18}};

static const std::array<std::array<int, 52>, 3> tc0{{{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
                                                       1, 1, 1, 2, 2, 2, 2, 3, 3, 3,
                                                       4, 4, 4, 5, 6, 6, 7, 8, 9, 10,
                                                       11, 13}},
                                                     {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                                       1, 2, 2, 2, 2, 3, 3, 3, 4, 4,
                                                       5, 5, 6, 7, 8, 8, 10, 11, 12, 13,
                                                       15, 17}},
                                                     {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
                                                       1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
                                                       2, 3, 3, 3, 4, 4, 4, 5, 6, 6,
                                                       7, 8, 9, 10, 11, 13, 14, 16, 18, 20,
                                                       23, 25}}}};

void deblocking_filter(std::vector<MacroBlock>&, Frame&);
void deblock_Y_vertical(int, MacroBlock&, std::vector<MacroBlock>&, Frame&);
void deblock_Y_horizontal(int, MacroBlock&, std::vector<MacroBlock>&, Frame&);
void filter_Y(int, int, int&, int&, int&, int&, int&, int&,int&, int&);
void deblock_Cr_Cb_vertical(int, MacroBlock&, std::vector<MacroBlock>&, Frame&);
void deblock_Cr_Cb_horizontal(int, MacroBlock&, std::vector<MacroBlock>&, Frame&);
void filter_Cr_Cb(int, int, int&, int&, int&, int&, int&, int&,int&, int&);

int clip1(int);
int clip3(int, int, int);

#endif
