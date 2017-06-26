#ifndef INTRA
#define INTRA

#include <array>
#include <vector>
#include <tuple>
#include <numeric>
#include <algorithm>
#include <functional>
#include <experimental/optional>

#include "block.h"

class Predictor {
public:
    std::vector<int> pred_pel;
    bool up_available;
    bool left_available;
    bool up_right_available;
    bool all_available;

    Predictor(int size): up_available(false), left_available(false), up_right_available(false), all_available(false) {
      switch (size) {
        case 4:
          this->pred_pel.reserve(13);
          break;
        case 8:
          this->pred_pel.reserve(17);
          break;
        case 16:
          this->pred_pel.reserve(33);
          break;
      }
    }
};

using CopyBlock4x4 = std::array<int, 16>;

enum class Intra4x4Mode {
  VERTICAL,
  HORIZONTAL,
  DC,
  DOWNLEFT,
  DOWNRIGHT,
  VERTICALRIGHT,
  HORIZONTALDOWN,
  VERTICALLEFT,
  HORIZONTALUP,
};

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

std::tuple<int, Intra4x4Mode> intra4x4(Block4x4, std::experimental::optional<Block4x4>, std::experimental::optional<Block4x4>,
                                        std::experimental::optional<Block4x4>, std::experimental::optional<Block4x4>);
void get_intra4x4(CopyBlock4x4&, const Predictor&, const Intra4x4Mode);
void intra4x4_vertical(CopyBlock4x4&, const Predictor&);
void intra4x4_horizontal(CopyBlock4x4&, const Predictor&);
void intra4x4_dc(CopyBlock4x4&, const Predictor&);
void intra4x4_downleft(CopyBlock4x4&, const Predictor&);
void intra4x4_downright(CopyBlock4x4&, const Predictor&);
void intra4x4_verticalright(CopyBlock4x4&, const Predictor&);
void intra4x4_horizontaldown(CopyBlock4x4&, const Predictor&);
void intra4x4_verticalleft(CopyBlock4x4&, const Predictor&);
void intra4x4_horizontalup(CopyBlock4x4&, const Predictor&);
Predictor get_intra4x4_predictor(std::experimental::optional<Block4x4>, std::experimental::optional<Block4x4>,
                                  std::experimental::optional<Block4x4>, std::experimental::optional<Block4x4>);
void intra4x4_reconstruct(Block4x4, std::experimental::optional<Block4x4>, std::experimental::optional<Block4x4>,
                            std::experimental::optional<Block4x4>, std::experimental::optional<Block4x4>, const Intra4x4Mode);

std::tuple<int, Intra16x16Mode> intra16x16(Block16x16&, std::experimental::optional<std::reference_wrapper<Block16x16>>, std::experimental::optional<std::reference_wrapper<Block16x16>>, std::experimental::optional<std::reference_wrapper<Block16x16>>);
void get_intra16x16(Block16x16&, const Predictor&, const Intra16x16Mode);
void intra16x16_vertical(Block16x16&, const Predictor&);
void intra16x16_horizontal(Block16x16&, const Predictor&);
void intra16x16_dc(Block16x16&, const Predictor&);
void intra16x16_plane(Block16x16&, const Predictor&);
Predictor get_intra16x16_predictor(std::experimental::optional<std::reference_wrapper<Block16x16>>, std::experimental::optional<std::reference_wrapper<Block16x16>>, std::experimental::optional<std::reference_wrapper<Block16x16>>);
void intra16x16_reconstruct(Block16x16&, std::experimental::optional<std::reference_wrapper<Block16x16>>, std::experimental::optional<std::reference_wrapper<Block16x16>>, std::experimental::optional<std::reference_wrapper<Block16x16>>, const Intra16x16Mode);

std::tuple<int, IntraChromaMode> intra8x8_chroma(Block8x8&, std::experimental::optional<std::reference_wrapper<Block8x8>>, std::experimental::optional<std::reference_wrapper<Block8x8>>, std::experimental::optional<std::reference_wrapper<Block8x8>>,
                                                  Block8x8&, std::experimental::optional<std::reference_wrapper<Block8x8>>, std::experimental::optional<std::reference_wrapper<Block8x8>>, std::experimental::optional<std::reference_wrapper<Block8x8>>);
void get_intra8x8_chroma(Block8x8&, const Predictor&, const IntraChromaMode);
void intra8x8_chroma_dc(Block8x8&, const Predictor&);
void intra8x8_chroma_horizontal(Block8x8&, const Predictor&);
void intra8x8_chroma_vertical(Block8x8&, const Predictor&);
void intra8x8_chroma_plane(Block8x8&, const Predictor&);
Predictor get_intra8x8_chroma_predictor(std::experimental::optional<std::reference_wrapper<Block8x8>>, std::experimental::optional<std::reference_wrapper<Block8x8>>, std::experimental::optional<std::reference_wrapper<Block8x8>>);
void intra8x8_chroma_reconstruct(Block8x8&, std::experimental::optional<std::reference_wrapper<Block8x8>>, std::experimental::optional<std::reference_wrapper<Block8x8>>, std::experimental::optional<std::reference_wrapper<Block8x8>>, const IntraChromaMode);

#endif
