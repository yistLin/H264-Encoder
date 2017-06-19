#ifndef INTRA
#define INTRA

#include <array>
#include <numeric>
#include <algorithm>
#include <functional>
#include <experimental/optional>

#include "block.h"

using Predictor16x16 = std::array<int, 33>;
using Predictor8x8 = std::array<int, 17>;

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

Intra16x16Mode intra16x16(Block16x16&, std::experimental::optional<std::reference_wrapper<Block16x16>>, std::experimental::optional<std::reference_wrapper<Block16x16>>, std::experimental::optional<std::reference_wrapper<Block16x16>>);
void get_intra16x16(Block16x16&, const Predictor16x16&, const Intra16x16Mode);
void intra16x16_vertical(Block16x16&, const Predictor16x16&);
void intra16x16_horizontal(Block16x16&, const Predictor16x16&);
void intra16x16_dc(Block16x16&, const Predictor16x16&);
void intra16x16_plane(Block16x16&, const Predictor16x16&);
Predictor16x16 get_intra16x16_predictor(const Block16x16&, const Block16x16&, const Block16x16&);

IntraChromaMode intra8x8_chroma(Block8x8&, const Block8x8&, const Block8x8&, const Block8x8&);
void get_intra8x8_chroma(Block8x8&, const Predictor8x8&, const IntraChromaMode);
void intra8x8_chroma_vertical(Block8x8&, const Predictor8x8&);
void intra8x8_chroma_horizontal(Block8x8&, const Predictor8x8&);
void intra8x8_chroma_dc(Block8x8&, const Predictor8x8&);
void intra8x8_chroma_plane(Block8x8&, const Predictor8x8&);
Predictor8x8 get_intra8x8_chroma_predictor(const Block8x8&, const Block8x8&, const Block8x8&);

#endif