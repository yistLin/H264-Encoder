#include "frame_encode.h"

Log f_logger("Frame encode");

void encode_I_frame(Frame& frame) {
  // decoded Y blocks for intra prediction
  std::vector<MacroBlock> decoded_blocks;
  decoded_blocks.reserve(frame.mbs.size());

  int mb_no = 0;
  for (auto& mb : frame.mbs) {
    f_logger.log(Level::DEBUG, "mb #" + std::to_string(mb_no++));
    decoded_blocks.push_back(mb);
    MacroBlock origin_block = mb;

    int error_luma = encode_Y_block(mb, decoded_blocks, frame);
    int error_chroma = encode_Cr_Cb_block(mb, decoded_blocks, frame);

    if (error_luma > 2000 || error_chroma > 1000) {
      f_logger.log(Level::VERBOSE, "error exceeded: luma " + std::to_string(error_luma) + " chroma: " + std::to_string(error_chroma));
      mb = origin_block;
      decoded_blocks.back() = origin_block;
    }
  }

  // in-loop deblocking filter
  deblocking_filter(decoded_blocks, frame);
}

int encode_Y_block(MacroBlock& mb, std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
  // temp marcoblock for choosing two predicitons
  MacroBlock temp_block = mb;
  MacroBlock temp_decoded_block = mb;

  // perform intra16x16 prediction
  int error_intra16x16 = encode_Y_intra16x16_block(mb, decoded_blocks, frame);

  // perform intra4x4 prediction
  int error_intra4x4 = 0;
  for (int i = 0; i != 16; i++)
    error_intra4x4 += encode_Y_intra4x4_block(i, temp_block, temp_decoded_block, decoded_blocks, frame);

  // compare the error of two predictions
  if (error_intra4x4 < error_intra16x16) {
    mb = temp_block;
    decoded_blocks.at(mb.mb_index) = temp_decoded_block;
    std::string mode = "\tmode:";
    for (auto& m : mb.intra4x4_Y_mode)
      mode += " " + std::to_string(static_cast<int>(m));
    f_logger.log(Level::DEBUG, "luma intra4x4\terror: " + std::to_string(error_intra4x4) + mode);

    return error_intra4x4;
  } else {
    f_logger.log(Level::DEBUG, "luma intra16x16\terror: " + std::to_string(error_intra16x16) + "\tmode: " + std::to_string(static_cast<int>(mb.intra16x16_Y_mode)));

    return error_intra16x16;
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
  int temp_pos = MacroBlock::convert_table[cur_pos];

  auto get_4x4_block = [&](int index, int pos) {
    if (index == -1)
      return std::experimental::optional<Block4x4>();
    else if (index == mb.mb_index)
      return std::experimental::optional<Block4x4>(decoded_block.get_Y_4x4_block(pos));
    else
      return std::experimental::optional<Block4x4>(decoded_blocks.at(index).get_Y_4x4_block(pos));
  };

  auto get_UL_4x4_block = [&]() {
    int index, pos;
    if (temp_pos == 0) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_UL);
      pos = 15;
    } else if (1 <= temp_pos && temp_pos <= 3) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_U);
      pos = 11 + temp_pos;
    } else if (temp_pos % 4 == 0) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
      pos = temp_pos - 1;
    } else {
      index = mb.mb_index;
      pos = temp_pos - 5;
    }

    return get_4x4_block(index, MacroBlock::convert_table[pos]);
  };

  auto get_U_4x4_block = [&]() {
    int index, pos;
    if (0 <= temp_pos && temp_pos <= 3) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_U);
      pos = 12 + temp_pos;
    } else {
      index = mb.mb_index;
      pos = temp_pos - 4;
    }

    return get_4x4_block(index, MacroBlock::convert_table[pos]);
  };

  auto get_UR_4x4_block = [&]() {
    int index, pos;
    if (temp_pos == 3) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_UR);
      pos = 12;
    } else if (temp_pos == 5 || temp_pos == 13) {
      index = -1;
      pos = 0;
    } else if (0 <= temp_pos && temp_pos <= 2) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_U);
      pos = 1 + temp_pos;
    } else if ((temp_pos + 1) % 4 == 0) {
      index = -1;
      pos = 0;
    } else {
      index = mb.mb_index;
      pos = temp_pos - 3;
    }

    return get_4x4_block(index, MacroBlock::convert_table[pos]);
  };

  auto get_L_4x4_block = [&]() {
    int index, pos;
    if (temp_pos % 4 == 0) {
      index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
      pos = temp_pos + 3;
    } else {
      index = mb.mb_index;
      pos = temp_pos - 1;
    }

    return get_4x4_block(index, MacroBlock::convert_table[pos]);
  };

  int error = 0;
  Intra4x4Mode mode;
  std::tie(error, mode) = intra4x4(mb.get_Y_4x4_block(cur_pos),
                                   get_UL_4x4_block(),
                                   get_U_4x4_block(),
                                   get_UR_4x4_block(),
                                   get_L_4x4_block());

  mb.is_intra16x16 = false;
  mb.intra4x4_Y_mode.at(cur_pos) = mode;

  // QDCT
  qdct_luma4x4_intra(mb.get_Y_4x4_block(cur_pos));

  // reconstruct for later prediction
  auto temp_4x4 = decoded_block.get_Y_4x4_block(cur_pos);
  auto temp_mb = mb.get_Y_4x4_block(cur_pos);
  for (int i = 0; i != 16; i++)
    temp_4x4[i] = temp_mb[i];
  inv_qdct_luma4x4_intra(decoded_block.get_Y_4x4_block(cur_pos));
  intra4x4_reconstruct(decoded_block.get_Y_4x4_block(cur_pos),
                       get_UL_4x4_block(),
                       get_U_4x4_block(),
                       get_UR_4x4_block(),
                       get_L_4x4_block(),
                       mode);

  return error;
}

int encode_Cr_Cb_block(MacroBlock& mb, std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
  int error_intra8x8 = encode_Cr_Cb_intra8x8_block(mb, decoded_blocks, frame);
  f_logger.log(Level::DEBUG, "chroma intra8x8\terror: " + std::to_string(error_intra8x8) + "\tmode: " + std::to_string(static_cast<int>(mb.intra_Cr_Cb_mode)));

  return error_intra8x8;
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
