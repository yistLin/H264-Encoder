#include "frame_encode.h"

void encode_I_frame(Frame& frame) {
  // decoded Y blocks for intra prediction
  std::vector<Block16x16> decoded_Y_blocks;
  decoded_Y_blocks.reserve(frame.mbs.size());

  for (auto& mb : frame.mbs) {
    encode_Y_block(mb, decoded_Y_blocks, frame);

    // encode_Cr_Cb_block();
  }
}

void encode_Y_block(MacroBlock& mb, std::vector<Block16x16>& decoded_Y_blocks, Frame& frame) {
  decoded_Y_blocks.push_back(mb.Y);

  auto get_decoded_Y_block = [&](int direction) {
    int index = frame.get_neighbor_index(mb.mb_index, direction);
    if (index == -1)
      return std::experimental::optional<std::reference_wrapper<Block16x16>>();
    else
      return std::experimental::optional<std::reference_wrapper<Block16x16>>(decoded_Y_blocks.at(index));
  };

  Intra16x16Mode mode = intra16x16(mb.Y, get_decoded_Y_block(MB_NEIGHBOR_UL),
                                         get_decoded_Y_block(MB_NEIGHBOR_U),
                                         get_decoded_Y_block(MB_NEIGHBOR_L));

  mb.is_intra16x16 = true;
  mb.intra16x16_Y_mode = mode;

  // QDCT
  // IQDCT

  // decode
  // decode(decoded_Y_blocks.back(), mode, FLAG);
}
