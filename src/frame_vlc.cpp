#include "frame_vlc.h"

void vlc_frame(Frame& frame) {
  std::vector<std::array<int, 16>> nc_Y_table;
  nc_Y_table.reserve(frame.mbs.size());
  std::vector<std::array<int, 4>> nc_Cb_table;
  nc_Cb_table.reserve(frame.mbs.size());
  std::vector<std::array<int, 4>> nc_Cr_table;
  nc_Cr_table.reserve(frame.mbs.size());

  // int mb_no = 0;
  for (auto& mb : frame.mbs) {
    // f_logger.log(Level::DEBUG, "mb #" + std::to_string(mb_no++));
    std::array<int, 16> current_Y_table;
    std::array<int, 4> current_Cb_table;
    std::array<int, 4> current_Cr_table;
    nc_Y_table.push_back(current_Y_table);
    nc_Cb_table.push_back(current_Cb_table);
    nc_Cr_table.push_back(current_Cr_table);

    if (mb.is_I_PCM) {
      for (int i = 0; i != 16; i++)
        nc_Y_table.at(mb.mb_index)[i] = 16;
      for (int i = 0; i != 4; i++) {
        nc_Cb_table.at(mb.mb_index)[i] = 16;
        nc_Cr_table.at(mb.mb_index)[i] = 16;
      }

      continue;
    }

    if (mb.is_intra16x16)
      mb.bitstream += vlc_Y_DC(mb, nc_Y_table, frame);

    std::array<Bitstream, 4> temp_luma;
    for (int i = 0; i != 16; i++)
      temp_luma[i / 4] += vlc_Y(i, mb, nc_Y_table, frame);
    if (mb.is_intra16x16) {
      if (mb.coded_block_pattern_luma)
        for (int i = 0; i != 4; i++)
          mb.bitstream += temp_luma[i];
    } else {
      for (int i = 0; i != 4; i++)
        if (mb.coded_block_pattern_luma_4x4[i])
          mb.bitstream += temp_luma[i];
    }

    Bitstream temp_chroma_DC;
    Bitstream temp_chroma_AC;
    temp_chroma_DC += vlc_Cb_DC(mb);
    temp_chroma_DC += vlc_Cr_DC(mb);
    for (int i = 0; i != 4; i++)
      temp_chroma_AC += vlc_Cb_AC(i, mb, nc_Cb_table, frame);
    for (int i = 0; i != 4; i++)
      temp_chroma_AC += vlc_Cr_AC(i, mb, nc_Cr_table, frame);

    if (mb.coded_block_pattern_chroma_DC || mb.coded_block_pattern_chroma_AC)
      mb.bitstream += temp_chroma_DC;
    if (mb.coded_block_pattern_chroma_AC)
      mb.bitstream += temp_chroma_AC;
  }
}

Bitstream vlc_Y_DC(MacroBlock& mb, std::vector<std::array<int, 16>>& nc_Y_table, Frame& frame) {
  int nA_index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
  int nB_index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_U);

  int nC;
  if (nA_index != -1 && nB_index != -1)
    nC = (nc_Y_table.at(nA_index)[5] + nc_Y_table.at(nB_index)[10] + 1) >> 1;
  else if (nA_index != -1)
    nC = nc_Y_table.at(nA_index)[5];
  else if (nB_index != -1)
    nC = nc_Y_table.at(nB_index)[10];
  else
    nC = 0;

  Bitstream bitstream;
  int non_zero;
  std::tie(bitstream, non_zero) = cavlc_block4x4(mb.get_Y_DC_block(), nC, 16);

  return bitstream;
}

Bitstream vlc_Y(int cur_pos, MacroBlock& mb, std::vector<std::array<int, 16>>& nc_Y_table, Frame& frame) {
  int real_pos = MacroBlock::convert_table[cur_pos];

  int nA_index, nA_pos;
  if (real_pos % 4 == 0) {
    nA_index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
    nA_pos = real_pos + 3;
  } else {
    nA_index = mb.mb_index;
    nA_pos = real_pos - 1;
  }
  nA_pos = MacroBlock::convert_table[nA_pos];

  int nB_index, nB_pos;
  if (0 <= real_pos && real_pos <= 3) {
    nB_index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_U);
    nB_pos = 12 + real_pos;
  } else {
    nB_index = mb.mb_index;
    nB_pos = real_pos - 4;
  }
  nB_pos = MacroBlock::convert_table[nB_pos];

  int nC;
  if (nA_index != -1 && nB_index != -1)
    nC = (nc_Y_table.at(nA_index)[nA_pos] + nc_Y_table.at(nB_index)[nB_pos] + 1) >> 1;
  else if (nA_index != -1)
    nC = nc_Y_table.at(nA_index)[nA_pos];
  else if (nB_index != -1)
    nC = nc_Y_table.at(nB_index)[nB_pos];
  else
    nC = 0;

  Bitstream bitstream;
  int non_zero;
  if (mb.is_intra16x16)
    std::tie(bitstream, non_zero) = cavlc_block4x4(mb.get_Y_AC_block(cur_pos), nC, 15);
  else
    std::tie(bitstream, non_zero) = cavlc_block4x4(mb.get_Y_4x4_block(cur_pos), nC, 16);
  nc_Y_table.at(mb.mb_index)[cur_pos] = non_zero;

  if (non_zero != 0) {
    mb.coded_block_pattern_luma = true;
    mb.coded_block_pattern_luma_4x4[cur_pos / 4] = true;
  }

  return bitstream;
}

Bitstream vlc_Cb_DC(MacroBlock& mb) {
  Bitstream bitstream;
  int non_zero;
  std::tie(bitstream, non_zero) = cavlc_block2x2(mb.get_Cb_DC_block(), -1, 4);

  if (non_zero != 0)
    mb.coded_block_pattern_chroma_DC = true;

  return bitstream;
}

Bitstream vlc_Cr_DC(MacroBlock& mb) {
  Bitstream bitstream;
  int non_zero;
  std::tie(bitstream, non_zero) = cavlc_block2x2(mb.get_Cr_DC_block(), -1, 4);

  if (non_zero != 0)
    mb.coded_block_pattern_chroma_DC = true;

  return bitstream;
}

Bitstream vlc_Cb_AC(int cur_pos, MacroBlock& mb, std::vector<std::array<int, 4>>& nc_Cb_table, Frame& frame) {
  int nA_index, nA_pos;
  if (cur_pos % 2 == 0) {
    nA_index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
    nA_pos = cur_pos + 1;
  } else {
    nA_index = mb.mb_index;
    nA_pos = cur_pos - 1;
  }

  int nB_index, nB_pos;
  if (0 <= cur_pos && cur_pos <= 1) {
    nB_index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_U);
    nB_pos = cur_pos + 2;
  } else {
    nB_index = mb.mb_index;
    nB_pos = cur_pos - 2;
  }

  int nC;
  if (nA_index != -1 && nB_index != -1)
    nC = (nc_Cb_table.at(nA_index)[nA_pos] + nc_Cb_table.at(nB_index)[nB_pos] + 1) >> 1;
  else if (nA_index != -1)
    nC = nc_Cb_table.at(nA_index)[nA_pos];
  else if (nB_index != -1)
    nC = nc_Cb_table.at(nB_index)[nB_pos];
  else
    nC = 0;

  Bitstream bitstream;
  int non_zero;
  std::tie(bitstream, non_zero) = cavlc_block4x4(mb.get_Cb_AC_block(cur_pos), nC, 15);
  nc_Cb_table.at(mb.mb_index)[cur_pos] = non_zero;

  if (non_zero != 0)
    mb.coded_block_pattern_chroma_AC = true;

  return bitstream;
}

Bitstream vlc_Cr_AC(int cur_pos, MacroBlock& mb, std::vector<std::array<int, 4>>& nc_Cr_table, Frame& frame) {
  int nA_index, nA_pos;
  if (cur_pos % 2 == 0) {
    nA_index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
    nA_pos = cur_pos + 1;
  } else {
    nA_index = mb.mb_index;
    nA_pos = cur_pos - 1;
  }

  int nB_index, nB_pos;
  if (0 <= cur_pos && cur_pos <= 1) {
    nB_index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_U);
    nB_pos = cur_pos + 2;
  } else {
    nB_index = mb.mb_index;
    nB_pos = cur_pos - 2;
  }

  int nC;
  if (nA_index != -1 && nB_index != -1)
    nC = (nc_Cr_table.at(nA_index)[nA_pos] + nc_Cr_table.at(nB_index)[nB_pos] + 1) >> 1;
  else if (nA_index != -1)
    nC = nc_Cr_table.at(nA_index)[nA_pos];
  else if (nB_index != -1)
    nC = nc_Cr_table.at(nB_index)[nB_pos];
  else
    nC = 0;

  Bitstream bitstream;
  int non_zero;
  std::tie(bitstream, non_zero) = cavlc_block4x4(mb.get_Cr_AC_block(cur_pos), nC, 15);
  nc_Cr_table.at(mb.mb_index)[cur_pos] = non_zero;

  if (non_zero != 0)
    mb.coded_block_pattern_chroma_AC = true;

  return bitstream;
}
