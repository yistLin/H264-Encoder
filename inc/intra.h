#ifndef INTRA
#define INTRA

#include <array>

#include "block.h"

using Predictor = std::array<int, 33>;

enum class Intra16x16Mode {
  VERTICAL,
  HORIZONTAL,
  DC,
  PLANE
};

enum class IntraChromaMode {
  DC,
  HORIZONTAL,
  VERTICAL,
  PLANE
};

Intra16x16Mode intra16x16(Block16x16&, const Block16x16&, const Block16x16&, const Block16x16&);
void intra16x16_vertical(Block16x16&, const Predictor&);
void intra16x16_horizontal(Block16x16&, const Predictor&);
void intra16x16_dc(Block16x16&, const Predictor&);
void intra16x16_plane(Block16x16&, const Predictor&);
Predictor get16x16_predictor(const Block16x16&, const Block16x16&, const Block16x16&);

IntraChromaMode intra8x8_chroma(Block8x8&, const Block8x8&, const Block8x8&, const Block8x8&);

#endif