#include "frame_encode.h"

Log f_logger("Frame encode");

void encode_I_frame(Frame& frame) {
  // decoded Y blocks for intra prediction
  std::vector<MacroBlock> decoded_blocks;
  decoded_blocks.reserve(frame.mbs.size());

  for (auto& mb : frame.mbs) {
    decoded_blocks.push_back(mb);
    encode_Y_block(mb, decoded_blocks, frame);
    encode_Cr_Cb_block(mb, decoded_blocks, frame);

    break;
  }
}

void encode_Y_block(MacroBlock& mb, std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
  // temp marcoblock for choosing two predicitons
  MacroBlock temp_block = mb;
  MacroBlock temp_decoded_block = mb;

  // perform intra16x16 prediction
  int error_intra16x16 = encode_Y_intra16x16_block(mb, decoded_blocks, frame);
  f_logger.log(Level::DEBUG, "intra16x16 error: " + std::to_string(error_intra16x16));

  for (int i = 0; i != 16; i++)
    std::cerr << mb.get_Y_4x4_block(15)[i] << " ";
  std::cerr << std::endl;

  for (auto i : mb.get_Y_4x4_block(15))
    std::cerr << i << " ";
  std::cerr << std::endl;

  // perform intra4x4 prediction
  int error_intra4x4 = 0;
  for (int i = 0; i != 16; i++)
    error_intra4x4 += encode_Y_intra4x4_block(i, temp_block, temp_decoded_block, decoded_blocks, frame);
  f_logger.log(Level::DEBUG, "intra4x4 error: " + std::to_string(error_intra4x4));

  // compare the error of two predictions
  if (error_intra4x4 < error_intra16x16) {
    mb = temp_block;
    decoded_blocks.at(mb.mb_index) = temp_decoded_block;
  }
}

int encode_Y_intra16x16_block(MacroBlock& mb, std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
  auto get_decoded_Y_block = [&](int direction) {
    int index = frame.get_neighbor_index(mb.mb_index, direction);
    if (index == -1)
      return std::experimental::optional<std::reference_wrapper<Block16x16>>();
    else
      return std::experimental::optional<std::reference_wrapper<Block16x16>>(decoded_blocks.at(index).Y);
  };

  // apply intra prediction
  int error;
  Intra16x16Mode mode;
  std::tie(error, mode) = intra16x16(mb.Y, get_decoded_Y_block(MB_NEIGHBOR_UL),
                                           get_decoded_Y_block(MB_NEIGHBOR_U),
                                           get_decoded_Y_block(MB_NEIGHBOR_L));

  mb.is_intra16x16 = true;
  mb.intra16x16_Y_mode = mode;

  // QDCT
  qdct_luma16x16_intra(mb.Y);

  // reconstruct for later prediction
  decoded_blocks.at(mb.mb_index).Y = mb.Y;
  inv_qdct_luma16x16_intra(decoded_blocks.at(mb.mb_index).Y);
  intra16x16_reconstruct(decoded_blocks.at(mb.mb_index).Y,
                         get_decoded_Y_block(MB_NEIGHBOR_UL),
                         get_decoded_Y_block(MB_NEIGHBOR_U),
                         get_decoded_Y_block(MB_NEIGHBOR_L),
                         mode);

  return error;
}

int encode_Y_intra4x4_block(int cur_pos, MacroBlock& mb, MacroBlock& decoded_block, std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
  auto get_4x4_block = [&](int index, int pos) {
    if (index == -1)
      return std::experimental::optional<std::reference_wrapper<Block4x4>>();
    else if (index == mb.mb_index)
      return std::experimental::optional<std::reference_wrapper<Block4x4>>(decoded_block.get_Y_4x4_block(pos));
    else
      return std::experimental::optional<std::reference_wrapper<Block4x4>>(decoded_blocks.at(index).get_Y_4x4_block(pos));
  };

  auto get_UL_4x4_block = [&]() {
    int index, pos;
    if (cur_pos == 0) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_UL);
      pos = 15;
    } else if (1 <= cur_pos && cur_pos <= 3) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_U);
      pos = 11 + cur_pos;
    } else if (cur_pos % 4 == 0) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
      pos = cur_pos - 1;
    } else {
      index = mb.mb_index;
      pos = cur_pos - 5;
    }

    get_4x4_block(index, pos);
  };

  auto get_U_4x4_block = [&]() {
    int index, pos;
    if (0 <= cur_pos && cur_pos <= 3) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_U);
      pos = 12 + cur_pos;
    } else {
      index = mb.mb_index;
      pos = cur_pos - 4;
    }

    get_4x4_block(index, pos);
  };

  auto get_UR_4x4_block = [&]() {
    int index, pos;
    if (cur_pos == 3) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_UR);
      pos = 12;
    } else if (0 <= cur_pos && cur_pos <= 2) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_U);
      pos = 1 + cur_pos;
    } else if ((cur_pos + 1) % 4 == 0) {
      index = -1;
      pos = cur_pos - 7;
    } else {
      index = mb.mb_index;
      pos = cur_pos - 3;
    }

    get_4x4_block(index, pos);
  };

  auto get_L_4x4_block = [&]() {
    int index, pos;
    if (cur_pos % 4 == 0) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
      pos = cur_pos + 3;
    } else {
      index = mb.mb_index;
      pos = cur_pos - 1;
    }

    get_4x4_block(index, pos);
  };

  int error = 0;
  // Intra4x4Mode mode;
  // std::tie(error, mode) = intra4x4(mb.Y, get_UL_4x4_block(),
  //                                        get_U_4x4_block(),
  //                                        get_UR_4x4_block(),
  //                                        get_L_4x4_block());

  mb.is_intra16x16 = false;
  // mb.intra4x4_Y_mode.at(cur_pos) = mode;

  // QDCT
  // qdct(mb.get_Y_4x4_block(cur_pos));

  // reconstruct for later prediction
  decoded_block.Y = mb.Y;
  // inv_qdct(decoded_block.get_Y_4x4_block(cur_pos));
  // intra4x4_reconstruct(decoded_block.Y,
  //                      get_UL_4x4_block(),
  //                      get_U_4x4_block(),
  //                      get_UR_4x4_block(),
  //                      get_L_4x4_block(),
  //                      mode);

  return error;
}

void encode_Cr_Cb_block(MacroBlock& mb, std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
  encode_Cr_Cb_intra8x8_block(mb, decoded_blocks, frame);
}

int encode_Cr_Cb_intra8x8_block(MacroBlock& mb, std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
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

  int error;
  IntraChromaMode mode;
  std::tie(error, mode) = intra8x8_chroma(mb.Cr, get_decoded_Cr_block(MB_NEIGHBOR_UL),
                                                 get_decoded_Cr_block(MB_NEIGHBOR_U),
                                                 get_decoded_Cr_block(MB_NEIGHBOR_L),
                                          mb.Cb, get_decoded_Cb_block(MB_NEIGHBOR_UL),
                                                 get_decoded_Cb_block(MB_NEIGHBOR_U),
                                                 get_decoded_Cb_block(MB_NEIGHBOR_L));

  mb.intra_Cr_Cb_mode = mode;

  // QDCT
  qdct_chroma8x8_intra(mb.Cr);
  qdct_chroma8x8_intra(mb.Cb);

  // reconstruct for later prediction
  decoded_blocks.at(mb.mb_index).Cr = mb.Cr;
  inv_qdct_chroma8x8_intra(decoded_blocks.at(mb.mb_index).Cr);
  intra8x8_chroma_reconstruct(decoded_blocks.at(mb.mb_index).Cr,
                              get_decoded_Cr_block(MB_NEIGHBOR_UL),
                              get_decoded_Cr_block(MB_NEIGHBOR_U),
                              get_decoded_Cr_block(MB_NEIGHBOR_L),
                              mode);
  decoded_blocks.at(mb.mb_index).Cb = mb.Cb;
  inv_qdct_chroma8x8_intra(decoded_blocks.at(mb.mb_index).Cb);
  intra8x8_chroma_reconstruct(decoded_blocks.at(mb.mb_index).Cb,
                              get_decoded_Cb_block(MB_NEIGHBOR_UL),
                              get_decoded_Cb_block(MB_NEIGHBOR_U),
                              get_decoded_Cb_block(MB_NEIGHBOR_L),
                              mode);

  return error;
}
