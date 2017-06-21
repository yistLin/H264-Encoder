#include "frame_encode.h"

void encode_I_frame(Frame& frame) {
  // decoded Y blocks for intra prediction
  std::vector<MacroBlock> decoded_blocks;
  decoded_blocks.reserve(frame.mbs.size());

  for (auto& mb : frame.mbs) {
    decoded_blocks.push_back(mb);
    encode_Y_block(mb, decoded_blocks, frame);
    encode_Cr_Cb_block(mb, decoded_blocks, frame);
  }
}

void encode_Y_block(MacroBlock& mb, std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
  auto get_decoded_Y_block = [&](int direction) {
    int index = frame.get_neighbor_index(mb.mb_index, direction);
    if (index == -1)
      return std::experimental::optional<std::reference_wrapper<Block16x16>>();
    else
      return std::experimental::optional<std::reference_wrapper<Block16x16>>(decoded_blocks.at(index).Y);
  };

  // apply intra prediction
  Intra16x16Mode mode = intra16x16(mb.Y, get_decoded_Y_block(MB_NEIGHBOR_UL),
                                         get_decoded_Y_block(MB_NEIGHBOR_U),
                                         get_decoded_Y_block(MB_NEIGHBOR_L));

  mb.is_intra16x16 = true;
  mb.intra16x16_Y_mode = mode;

  decoded_blocks.at(mb.mb_index).Y = mb.Y;

  // QDCT & IQDCT
  qdct_luma16x16_intra(decoded_blocks.at(mb.mb_index).Y);
  inv_qdct_luma16x16_intra(decoded_blocks.at(mb.mb_index).Y);

  // reconstruct for later prediction
  intra16x16_reconstruct(decoded_blocks.at(mb.mb_index).Y,
                         get_decoded_Y_block(MB_NEIGHBOR_UL),
                         get_decoded_Y_block(MB_NEIGHBOR_U),
                         get_decoded_Y_block(MB_NEIGHBOR_L),
                         mode);
}

void encode_Cr_Cb_block(MacroBlock& mb, std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
  auto get_decoded_Cr_block = [&](int direction) {
    int index = frame.get_neighbor_index(mb.mb_index, direction);
    if (index == -1)
      return std::experimental::optional<std::reference_wrapper<Block8x8>>();
    else
      return std::experimental::optional<std::reference_wrapper<Block8x8>>(decoded_blocks.at(index).Cr);
  };

  auto get_decoded_Cb_block = [&](int direction) {
    int index = frame.get_neighbor_index(mb.mb_index, direction);
    if (index == -1)
      return std::experimental::optional<std::reference_wrapper<Block8x8>>();
    else
      return std::experimental::optional<std::reference_wrapper<Block8x8>>(decoded_blocks.at(index).Cb);
  };

  IntraChromaMode mode = intra8x8_chroma(mb.Cr, get_decoded_Cr_block(MB_NEIGHBOR_UL),
                                                get_decoded_Cr_block(MB_NEIGHBOR_U),
                                                get_decoded_Cr_block(MB_NEIGHBOR_L),
                                         mb.Cb, get_decoded_Cb_block(MB_NEIGHBOR_UL),
                                                get_decoded_Cb_block(MB_NEIGHBOR_U),
                                                get_decoded_Cb_block(MB_NEIGHBOR_L));

  mb.intra_Cr_Cb_mode = mode;

  decoded_blocks.at(mb.mb_index).Cr = mb.Cr;
  decoded_blocks.at(mb.mb_index).Cb = mb.Cb;

  // QDCT & IQDCT
  // qdct_luma16x16_intra(decoded_blocks.at(mb.mb_index).Y);
  // inv_qdct_luma16x16_intra(decoded_blocks.at(mb.mb_index).Y);

  // reconstruct for later prediction
  intra8x8_chroma_reconstruct(decoded_blocks.at(mb.mb_index).Cr,
                              get_decoded_Cr_block(MB_NEIGHBOR_UL),
                              get_decoded_Cr_block(MB_NEIGHBOR_U),
                              get_decoded_Cr_block(MB_NEIGHBOR_L),
                              mode);
  intra8x8_chroma_reconstruct(decoded_blocks.at(mb.mb_index).Cb,
                              get_decoded_Cb_block(MB_NEIGHBOR_UL),
                              get_decoded_Cb_block(MB_NEIGHBOR_U),
                              get_decoded_Cb_block(MB_NEIGHBOR_L),
                              mode);
}
