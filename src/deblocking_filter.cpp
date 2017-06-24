#include "deblocking_filter.h"

void deblocking_filter(std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
  std::array<int, 16> vertical_Y_order{{0, 2, 8, 10, 1, 3, 9, 11, 4, 6, 12, 14, 5, 7, 13, 15}};
  std::array<int, 16> horizontal_Y_order{{0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15}};
  std::array<int, 4> vertical_Cr_Cb_order{{0, 2, 1, 3}};
  std::array<int, 4> horizontal_Cr_Cb_order{{0, 1, 2, 3}};
  for (auto& mb : decoded_blocks) {
    for (int i = 0; i != 16; i++) {
      deblock_Y_vertical(vertical_Y_order[i], mb, decoded_blocks, frame);
      deblock_Y_horizontal(horizontal_Y_order[i], mb, decoded_blocks, frame);
    }
    for (int i = 0; i != 4; i++) {
      deblock_Cr_Cb_vertical(vertical_Cr_Cb_order[i], mb, decoded_blocks, frame);
      deblock_Cr_Cb_horizontal(horizontal_Cr_Cb_order[i], mb, decoded_blocks, frame);
    }
  }
}

void deblock_Y_vertical(int cur_pos, MacroBlock& mb, std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
  int real_pos = MacroBlock::convert_table[cur_pos];
  bool is_boundary = false;

  // get the left 4x4 block index
  int index, temp_pos;
  if (real_pos % 4 == 0) {
    is_boundary = true;
    index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
    temp_pos = real_pos + 3;
  } else {
    index = mb.mb_index;
    temp_pos = real_pos - 1;
  }
  temp_pos = MacroBlock::convert_table[temp_pos];

  if (index == -1)
    return;

  Block4x4 blockQ = decoded_blocks.at(mb.mb_index).get_Y_4x4_block(cur_pos);
  Block4x4 blockP = decoded_blocks.at(mb.mb_index).get_Y_4x4_block(temp_pos);
  if (index != mb.mb_index)
    blockP = decoded_blocks.at(index).get_Y_4x4_block(temp_pos);

  // is_intra
  int bs;
  if (is_boundary)
    bs = 4;
  else
    bs = 3;

  int qPav = (LUMA_QP + LUMA_QP) >> 1;
  for (int i = 0; i != 4; i++)
    filter_Y(bs, qPav, blockP[i * 4], blockP[i * 4 + 1], blockP[i * 4 + 2], blockP[i * 4 + 3], blockQ[i * 4], blockQ[i * 4 + 1], blockQ[i * 4 + 2], blockQ[i * 4 + 3]);
}

void deblock_Y_horizontal(int cur_pos, MacroBlock& mb, std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
  int real_pos = MacroBlock::convert_table[cur_pos];
  bool is_boundary = false;

  // get the up 4x4 block index
  int index, temp_pos;
  if (0 <= real_pos && real_pos <= 3) {
    is_boundary = true;
    index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
    temp_pos = 12 + real_pos;
  } else {
    index = mb.mb_index;
    temp_pos = real_pos - 4;
  }
  temp_pos = MacroBlock::convert_table[temp_pos];

  if (index == -1)
    return;

  Block4x4 blockQ = decoded_blocks.at(mb.mb_index).get_Y_4x4_block(cur_pos);
  Block4x4 blockP = decoded_blocks.at(mb.mb_index).get_Y_4x4_block(temp_pos);
  if (index != mb.mb_index)
    blockP = decoded_blocks.at(index).get_Y_4x4_block(temp_pos);

  // is_intra
  int bs;
  if (is_boundary)
    bs = 4;
  else
    bs = 3;

  int qPav = (LUMA_QP + LUMA_QP) >> 1;
  for (int i = 0; i != 4; i++)
    filter_Y(bs, qPav, blockP[i * 4], blockP[i * 4 + 1], blockP[i * 4 + 2], blockP[i * 4 + 3], blockQ[i * 4], blockQ[i * 4 + 1], blockQ[i * 4 + 2], blockQ[i * 4 + 3]);
}

void deblock_Cr_Cb_vertical(int cur_pos, MacroBlock& mb, std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
  bool is_boundary = false;

  // get the left 4x4 block index
  int index, temp_pos;
  if (cur_pos % 2 == 0) {
    is_boundary = true;
    index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
    temp_pos = cur_pos + 1;
  } else {
    index = mb.mb_index;
    temp_pos = cur_pos - 1;
  }

  if (index == -1)
    return;

  Block4x4 blockQ_Cr = decoded_blocks.at(mb.mb_index).get_Cr_4x4_block(cur_pos);
  Block4x4 blockP_Cr = decoded_blocks.at(mb.mb_index).get_Cr_4x4_block(temp_pos);
  Block4x4 blockQ_Cb = decoded_blocks.at(mb.mb_index).get_Cb_4x4_block(cur_pos);
  Block4x4 blockP_Cb = decoded_blocks.at(mb.mb_index).get_Cb_4x4_block(temp_pos);
  if (index != mb.mb_index) {
    blockP_Cr = decoded_blocks.at(index).get_Cr_4x4_block(temp_pos);
    blockP_Cb = decoded_blocks.at(index).get_Cb_4x4_block(temp_pos);
  }

  // is_intra
  int bs;
  if (is_boundary)
    bs = 4;
  else
    bs = 3;

  int qPav = (CHROMA_QP + CHROMA_QP) >> 1;
  for (int i = 0; i != 4; i++) {
    filter_Cr_Cb(bs, qPav, blockP_Cr[i * 4], blockP_Cr[i * 4 + 1], blockP_Cr[i * 4 + 2], blockP_Cr[i * 4 + 3], blockQ_Cr[i * 4], blockQ_Cr[i * 4 + 1], blockQ_Cr[i * 4 + 2], blockQ_Cr[i * 4 + 3]);
    filter_Cr_Cb(bs, qPav, blockP_Cb[i * 4], blockP_Cb[i * 4 + 1], blockP_Cb[i * 4 + 2], blockP_Cb[i * 4 + 3], blockQ_Cb[i * 4], blockQ_Cb[i * 4 + 1], blockQ_Cb[i * 4 + 2], blockQ_Cb[i * 4 + 3]);
  }
}

void deblock_Cr_Cb_horizontal(int cur_pos, MacroBlock& mb, std::vector<MacroBlock>& decoded_blocks, Frame& frame) {
  bool is_boundary = false;

  // get the up 4x4 block index
  int index, temp_pos;
  if (0 <= cur_pos && cur_pos <= 1) {
    is_boundary = true;
    index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
    temp_pos = cur_pos + 2;
  } else {
    index = mb.mb_index;
    temp_pos = cur_pos - 2;
  }

  if (index == -1)
    return;

  Block4x4 blockQ_Cr = decoded_blocks.at(mb.mb_index).get_Cr_4x4_block(cur_pos);
  Block4x4 blockP_Cr = decoded_blocks.at(mb.mb_index).get_Cr_4x4_block(temp_pos);
  Block4x4 blockQ_Cb = decoded_blocks.at(mb.mb_index).get_Cb_4x4_block(cur_pos);
  Block4x4 blockP_Cb = decoded_blocks.at(mb.mb_index).get_Cb_4x4_block(temp_pos);
  if (index != mb.mb_index) {
    blockP_Cr = decoded_blocks.at(index).get_Cr_4x4_block(temp_pos);
    blockP_Cb = decoded_blocks.at(index).get_Cb_4x4_block(temp_pos);
  }

  // is_intra
  int bs;
  if (is_boundary)
    bs = 4;
  else
    bs = 3;

  int qPav = (CHROMA_QP + CHROMA_QP) >> 1;
  for (int i = 0; i != 4; i++) {
    filter_Cr_Cb(bs, qPav, blockP_Cr[i * 4], blockP_Cr[i * 4 + 1], blockP_Cr[i * 4 + 2], blockP_Cr[i * 4 + 3], blockQ_Cr[i * 4], blockQ_Cr[i * 4 + 1], blockQ_Cr[i * 4 + 2], blockQ_Cr[i * 4 + 3]);
    filter_Cr_Cb(bs, qPav, blockP_Cb[i * 4], blockP_Cb[i * 4 + 1], blockP_Cb[i * 4 + 2], blockP_Cb[i * 4 + 3], blockQ_Cb[i * 4], blockQ_Cb[i * 4 + 1], blockQ_Cb[i * 4 + 2], blockQ_Cb[i * 4 + 3]);
  }
}

int clip1(int value) {
  return clip3(0, 255, value);
}

int clip3(int l, int r, int value) {
  if (value < l)
    return l;
  if (value > r)
    return r;
  return value;
}

void filter_Y(int bs, int qPav, int& p3, int& p2, int& p1, int& p0, int& q0, int& q1,int& q2, int& q3) {
  int indexA = clip3(0, 51, qPav + 0);
  int indexB = clip3(0, 51, qPav + 0);

  int a = alpha[indexA];
  int b = beta[indexB];

  int ap = std::abs(p2 - p0);
  int aq = std::abs(q2 - q0);

  if (!(bs != 0 && std::abs(p0 - q0) < a && std::abs(p1 - p0) < b && std::abs(q1 - q0) < b))
    return;

  int t_p2 = p2, t_p1 = p1, t_p0 = p0;
  int t_q0 = q0, t_q1 = q1, t_q2 = q2;

  if (bs == 4) {
    if (ap < b && std::abs(p0 - q0) < ((a >> 2) + 2)) {
      t_p0 = (p2 + 2 * p1 + 2 * p0 + 2 * q0 + q1 + 4) >> 3;
      t_p1 = (p2 + p1 + p0 + q0 + 2) >> 2;
      t_p2 = (2 * p3 + 3 * p2 + p1 + p0 + q0 + 4) >> 3;
    } else {
      t_p0 =(2 * p1 + p0 + q1 + 2) >> 2;
    }
    if (aq < b && std::abs(p0 - q0) < ((a >> 2) + 2)) {
      t_q0 = (p1 + 2 * p0 + 2 * q0 + 2 * q1 + q2 + 4 )>> 3;
      t_q1 = (p0 + q0 + q1 + q2 + 2) >> 2;
      t_q2 = (2 * q3 + 3 * q2 + q1 + q0 + p0 + 4 ) >> 3;
    } else {
      t_q0 = (2 * q1 + q0 + p1 + 2 ) >> 2;
    }
  } else {
    int c = tc0[bs - 1][indexA];
    int tc = c + ((ap < b) ? 1 : 0) + ((aq < b) ? 1 : 0);
    int delta = clip3(-tc, tc, (((q0 - p0) << 2) + (p1 - q1) + 4) >> 3);
    t_p0 = clip1(p0 + delta);
    t_q0 = clip1(q0 - delta);
    if (ap < b)
      t_p1 = p1 + clip3(-c, c, (p2 + ((p0 + q0 + 1) >> 1) - (p1 << 1)) >> 1);
    if (aq < b)
      t_q1 = q1 + clip3(-c, c, (q2 + ((p0 + q0 + 1) >> 1) - (q1 << 1)) >> 1);
  }

  p2 = t_p2, p1 = t_p1, p0 = t_p0;
  q0 = t_q0, q1 = t_q1, q2 = t_q2;
}


void filter_Cr_Cb(int bs, int qPav, int& p3, int& p2, int& p1, int& p0, int& q0, int& q1,int& q2, int& q3) {
  int indexA = clip3(0, 51, qPav + 0);
  int indexB = clip3(0, 51, qPav + 0);

  int a = alpha[indexA];
  int b = beta[indexB];

  if (!(bs != 0 && std::abs(p0 - q0) < a && std::abs(p1 - p0) < b && std::abs(q1 - q0) < b))
    return;

  int t_p0 = p0;
  int t_q0 = q0;

  if (bs == 4) {
    t_p0 = (2 * p1 + p0 + q1 + 2) >> 2;
    t_q0 = (2 * q1 + q0 + p1 + 2) >> 2;
  } else {
    int c = tc0[bs - 1][indexA];
    int tc = c + 1;
    int delta = clip3(-tc, tc, (((q0 - p0) << 2) + (p1 - q1) + 4) >> 3);
    t_p0 = clip1(p0 + delta);
    t_q0 = clip1(q0 - delta);
  }

  p0 = t_p0;
  q0 = t_q0;
}
