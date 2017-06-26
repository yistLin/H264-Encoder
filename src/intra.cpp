#include "intra.h"

/* Clip function
 */
template <typename T>
T clip(const T& n, const T& lower, const T& upper) {
  return std::max(lower, std::min(n, upper));
}

/* Input iterators
 * return summation of absolute difference (SAD)
 */
template<class InputIt1, class InputIt2, class OutputIt>
int SAD(InputIt1 first1, InputIt1 last1, InputIt2 first2, OutputIt result) {
  int sad = 0;
  int diff;
  while (first1 != last1) {
    diff = (*first1++ - *first2++);
    *result++ = diff;
    sad += (diff > 0)? diff: -diff;
  }
  return sad;
}

/* Input 4x4 block and its neighbors
 * do intra4x4 prediction which has 9 modes
 * overwrite residual on input block
 * return the least cost mode
 */
std::tuple<int, Intra4x4Mode> intra4x4(Block4x4 block,
  std::experimental::optional<Block4x4> ul,
  std::experimental::optional<Block4x4> u,
  std::experimental::optional<Block4x4> ur,
  std::experimental::optional<Block4x4> l) {

  // Get predictors
  Predictor predictor = get_intra4x4_predictor(ul, u, ur, l);

  int mode;
  Intra4x4Mode best_mode;
  CopyBlock4x4 pred, residual;
  int min_sad = (1 << 15), sad;
  // Run all modes to get least residual
  for (mode = 0; mode < 9; mode++) {

    if ((!predictor.up_available   && (Intra4x4Mode::VERTICAL   == static_cast<Intra4x4Mode>(mode))) ||
        (!predictor.left_available && (Intra4x4Mode::HORIZONTAL == static_cast<Intra4x4Mode>(mode))) ||
        ((!predictor.up_available || !predictor.up_right_available) && (Intra4x4Mode::DOWNLEFT == static_cast<Intra4x4Mode>(mode))) ||
        ((!predictor.up_available || !predictor.left_available) && (Intra4x4Mode::DOWNRIGHT == static_cast<Intra4x4Mode>(mode))) ||
        ((!predictor.up_available || !predictor.left_available) && (Intra4x4Mode::VERTICALRIGHT == static_cast<Intra4x4Mode>(mode))) ||
        ((!predictor.up_available || !predictor.left_available) && (Intra4x4Mode::HORIZONTALDOWN == static_cast<Intra4x4Mode>(mode))) ||
        ((!predictor.up_available || !predictor.up_right_available) && (Intra4x4Mode::VERTICALLEFT == static_cast<Intra4x4Mode>(mode))) ||
        (!predictor.left_available && (Intra4x4Mode::HORIZONTALUP == static_cast<Intra4x4Mode>(mode)))) {
      continue;
    }

    get_intra4x4(pred, predictor, static_cast<Intra4x4Mode>(mode));

    sad = SAD(block.begin(), block.end(), pred.begin(), pred.begin());
    if (sad < min_sad) {
      min_sad = sad;
      best_mode = static_cast<Intra4x4Mode>(mode);
      std::copy(pred.begin(), pred.end(), residual.begin());
    }
  }

  // use operator = instead of std::copy which use *iter to deal with assignment
  for (int i = 0; i < 16; i++) {
    block[i] = residual[i];
  }

  return std::make_tuple(min_sad, best_mode);
}

/* Input residual, neighbors and prediction mode
 * overwrite reconstructed block on resudual
 */
void intra4x4_reconstruct(Block4x4 block,
  std::experimental::optional<Block4x4> ul,
  std::experimental::optional<Block4x4> u,
  std::experimental::optional<Block4x4> ur,
  std::experimental::optional<Block4x4> l,
  const Intra4x4Mode mode) {

  CopyBlock4x4 pred;
  Predictor predictor = get_intra4x4_predictor(ul, u, ur, l);
  get_intra4x4(pred, predictor, mode);

  // std::transform(block.begin(), block.end(), pred.begin(), block.begin(), std::plus<int>());
  for (int i = 0; i < 16; i++) {
    block[i] += pred[i];
  }
}

/* Input predictors and mode
 * write intra16x16 prediction on pred
 */
void get_intra4x4(CopyBlock4x4& pred, const Predictor& p, const Intra4x4Mode mode) {
  switch (mode) {
    case Intra4x4Mode::VERTICAL:
      intra4x4_vertical(pred, p);
      break;
    case Intra4x4Mode::HORIZONTAL:
      intra4x4_horizontal(pred, p);
      break;
    case Intra4x4Mode::DC:
      intra4x4_dc(pred, p);
      break;
    case Intra4x4Mode::DOWNLEFT:
      intra4x4_downleft(pred, p);
      break;
    case Intra4x4Mode::DOWNRIGHT:
      intra4x4_downright(pred, p);
      break;
    case Intra4x4Mode::VERTICALRIGHT:
      intra4x4_verticalright(pred, p);
      break;
    case Intra4x4Mode::HORIZONTALDOWN:
      intra4x4_horizontaldown(pred, p);
      break;
    case Intra4x4Mode::VERTICALLEFT:
      intra4x4_verticalleft(pred, p);
      break;
    case Intra4x4Mode::HORIZONTALUP:
      intra4x4_horizontalup(pred, p);
      break;
  }
}

void intra4x4_vertical(CopyBlock4x4& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  int i;
  for (i = 0; i < 4; i++) {
    std::copy_n(p.begin()+1, 4, pred.begin()+i*4);
  }
}

void intra4x4_horizontal(CopyBlock4x4& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  int i, j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      pred[i*4+j] = p[9+i];
    }
  }
}

void intra4x4_dc(CopyBlock4x4& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  int s1 = 0, s2 = 0, s = 0;
  int i;

  for (i = 1; i < 5; i++) {
    s1 += p[i];
  }

  for (i = 9; i < 13; i++) {
    s2 += p[i];
  }

  if (predictor.up_available && predictor.left_available) {
    s = s1 + s2;
  }
  else if (!predictor.up_available && predictor.left_available) {
    s = 2 * s2;
  }
  else if (predictor.up_available && !predictor.left_available) {
    s = 2 * s1;
  }

  s += 4;
  s >>= 3;

  if (!predictor.up_available && !predictor.left_available) {
    s = 128;
  }

  pred.fill(s);
}

void intra4x4_downleft(CopyBlock4x4& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  // hard code for speed
  // x + y = 0
  pred[0]  = ((p[1] + p[3] + (p[2] << 1) + 2) >> 2);
  // x + y = 1
  pred[1]  = pred[4]  = ((p[2] + p[4] + (p[3] << 1) + 2) >> 2);
  // x + y = 2
  pred[2]  = pred[5]  = pred[8]  = ((p[3] + p[5] + (p[4] << 1) + 2) >> 2);
  // x + y = 3
  pred[3]  = pred[6]  = pred[9]  = pred[12] = ((p[4] + p[6] + (p[5] << 1) + 2) >> 2);
  // x + y = 4
  pred[7]  = pred[10] = pred[13] = ((p[5] + p[7] + (p[6] << 1) + 2) >> 2);
  // x + y = 5
  pred[11] = pred[14] = ((p[6] + p[8] + (p[7] << 1) + 2) >> 2);
  // x + y = 6
  pred[15] = ((p[7] + 3 * p[8] + 2) >> 2);
}

void intra4x4_downright(CopyBlock4x4& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  // hard code for speed
  // x < y
  // y - x = 3
  pred[12] = ((p[12] + p[10] + (p[11] << 1) + 2) >> 2);
  // y - x = 2
  pred[8]  = pred[13] = ((p[11] + p[9] + (p[10] << 1) + 2) >> 2);
  // y - x = 1
  pred[4]  = pred[9]  = pred[14] = ((p[10] + p[1] + (p[9] << 1) + 2) >> 2);
  // x = y
  pred[0]  = pred[5]  = pred[10] = pred[15] = ((p[0] + p[9] + (p[1] << 1) + 2) >> 2);
  // x > y
  // x - y = 1
  pred[1]  = pred[6]  = pred[11] = ((p[0] + p[2] + (p[1] << 1) + 2) >> 2);
  // x - y = 2
  pred[2]  = pred[7]  = ((p[1] + p[3] + (p[2] << 1) + 2) >> 2);
  // x - y = 3
  pred[3]  = ((p[1] + p[3] + (p[2] << 1) + 2) >> 2);
}

void intra4x4_verticalright(CopyBlock4x4& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  // hard code for speed
  // zVR = 2 * x - y
  // zVR = 0
  pred[0]  = pred[9]  = ((p[0] + p[1] + 1) >> 1);
  // zVR = 2
  pred[1]  = pred[10] = ((p[1] + p[2] + 1) >> 1);
  // zVR = 4
  pred[2]  = pred[11] = ((p[2] + p[3] + 1) >> 1);
  // zVR = 6
  pred[3]  = ((p[3] + p[4] + 1) >> 1);
  // zVR = -1
  pred[4]  = pred[13] = ((p[1] + p[9] + (p[0] << 1) + 2) >> 2);
  // zVR = 1
  pred[5]  = pred[14] = ((p[0] + p[2] + (p[1] << 1) + 2) >> 2);
  // zVR = 3
  pred[6]  = pred[15] = ((p[1] + p[3] + (p[2] << 1) + 2) >> 2);
  // zVR = 5
  pred[7]  = ((p[2] + p[4] + (p[3] << 1) + 2) >> 2);
  // zVR = -2
  pred[8]  = ((p[0] + p[10] + (p[9] << 1) + 2) >> 2);
  // zVR = -3
  pred[12] = ((p[9] + p[11] + (p[10] << 1) + 2) >> 2);
}

void intra4x4_horizontaldown(CopyBlock4x4& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  // hard code for speed
  // zHD = 2 * y - x
  // zHD = 0
  pred[0]  = pred[6]  = ((p[0] + p[9] + 1) >> 1);
  // zHD = -1
  pred[1]  = pred[7]  = ((p[1] + p[9] + (p[0] << 1) + 2) >> 2);
  // zHD = -2
  pred[2]  = ((p[0] + p[2] + (p[1] << 1) + 2) >> 2);
  // zHD = -3
  pred[3]  = ((p[1] + p[3] + (p[2] << 1) + 2) >> 2);
  // zHD = 2
  pred[4]  = pred[10] = ((p[9] + p[10] + 1) >> 1);
  // zHD = 1
  pred[5]  = pred[11] = ((p[0] + p[11] + (p[10] << 1) + 2) >> 2);
  // zHD = 4
  pred[8]  = pred[14] = ((p[10] + p[11] + 1) >> 1);
  // zHD = 3
  pred[9]  = pred[15] = ((p[9] + p[11] + (p[10] << 1) + 2) >> 2);
  // zHD = 6
  pred[12] = ((p[11] + p[12] + 1) >> 1);
  // zHD = 5
  pred[13] = ((p[10] + p[12] + (p[11] << 1) + 2) >> 2);
}

void intra4x4_verticalleft(CopyBlock4x4& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  // hard code for speed
  pred[0]  = ((p[1] + p[2] + 1) >> 1);
  pred[1]  = pred[8]  = ((p[2] + p[3] + 1) >> 1);
  pred[2]  = pred[9]  = ((p[3] + p[4] + 1) >> 1);
  pred[3]  = pred[10] = ((p[4] + p[5] + 1) >> 1);
  pred[11] = ((p[5] + p[6] + 1) >> 1);
  pred[4]  = ((p[1] + p[3] + (p[2] << 1) + 2) >> 2);
  pred[5]  = pred[12] = ((p[2] + p[4] + (p[3] << 1) + 2) >> 2);
  pred[6]  = pred[13] = ((p[3] + p[5] + (p[4] << 1) + 2) >> 2);
  pred[7]  = pred[14] = ((p[4] + p[6] + (p[5] << 1) + 2) >> 2);
  pred[15] = ((p[5] + p[7] + (p[6] << 1) + 2) >> 2);
}

void intra4x4_horizontalup(CopyBlock4x4& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  // hard code for speed
  // zHU = x + 2 * y
  // zHU = 0
  pred[0]  = ((p[9] + p[10] + 1) >> 1);
  // zHU = 1
  pred[1]  = ((p[9] + p[11] + (p[10] << 1) + 2) >> 2);
  // zHU = 2
  pred[2]  = pred[4]  = ((p[10] + p[11] + 1) >> 1);
  // zHU = 3
  pred[3]  = pred[5]  = ((p[10] + p[12] + (p[11] << 1) + 2) >> 2);
  // zHU = 4
  pred[6]  = pred[8] = ((p[11] + p[12] + 1) >> 1);
  // zHU = 5
  pred[7]  = pred[9] = ((p[11] + (3 * p[12]) + 2) >> 2);
  // zHU > 5
  pred[12] = pred[10] = pred[11] =
  pred[13] = pred[14] = pred[15] = p[12];
}

/* Get intra16x16 predictors from neighbors
 * [0]: downmost and rightmost pixel of ul
 * [1..4]: downmost row of u
 * [5..8]: downmost row of ur
 * [9..12]: rightmost column of l
 */
Predictor get_intra4x4_predictor(
  std::experimental::optional<Block4x4> ul,
  std::experimental::optional<Block4x4> u,
  std::experimental::optional<Block4x4> ur,
  std::experimental::optional<Block4x4> l) {

  Predictor predictor(4);
  std::vector<int>& p = predictor.pred_pel;
  // Check whether neighbors are available
  if (u) {
    Block4x4& tmp = *u;
    std::copy_n(tmp.begin()+4*3, 4, p.begin()+1);
    predictor.up_available = true;
  }
  else {
    std::fill_n(p.begin()+1, 4, 128);
  }

  if (ur) {
    Block4x4& tmp = *ur;
    std::copy_n(tmp.begin()+4*3, 4, p.begin()+5);
    predictor.up_right_available = true;
  }
  else {
    std::fill_n(p.begin()+5, 4, p[4]);
  }

  if (l) {
    Block4x4& tmp = *l;
    for (int i = 0; i < 4; i++) {
      p[9+i] = tmp[i*4+3];
    }
    predictor.left_available = true;
  }
  else {
    std::fill_n(p.begin()+9, 4, 128);
  }

  if (predictor.up_available && predictor.left_available) {
    Block4x4& tmp = *ul;
    p[0] = tmp[15];
    predictor.all_available = true;
  }
  else {
    p[0] = 128;
  }

  return predictor;
}

/* Input 16x16 block and its neighbors
 * do intra16x16 prediction which has 4 modes
 * overwrite residual on input block
 * return the least cost mode
 */
std::tuple<int, Intra16x16Mode> intra16x16(Block16x16& block,
  std::experimental::optional<std::reference_wrapper<Block16x16>> ul,
  std::experimental::optional<std::reference_wrapper<Block16x16>> u,
  std::experimental::optional<std::reference_wrapper<Block16x16>> l) {

  // Get predictors
  Predictor predictor = get_intra16x16_predictor(ul, u, l);

  int mode;
  Intra16x16Mode best_mode;
  Block16x16 pred, residual;
  int min_sad = (1 << 15), sad;
  // Run all modes to get least residual
  for (mode = 0; mode < 4; mode++) {

    if ((!predictor.up_available   && (Intra16x16Mode::VERTICAL   == static_cast<Intra16x16Mode>(mode))) ||
        (!predictor.left_available && (Intra16x16Mode::HORIZONTAL == static_cast<Intra16x16Mode>(mode))) ||
        (!predictor.all_available  && (Intra16x16Mode::PLANE      == static_cast<Intra16x16Mode>(mode)))) {
      continue;
    }

    get_intra16x16(pred, predictor, static_cast<Intra16x16Mode>(mode));

    sad = SAD(block.begin(), block.end(), pred.begin(), pred.begin());
    if (sad < min_sad) {
      min_sad = sad;
      best_mode = static_cast<Intra16x16Mode>(mode);
      std::copy(pred.begin(), pred.end(), residual.begin());
    }
  }
  std::copy(residual.begin(), residual.end(), block.begin());

  return std::make_tuple(min_sad, best_mode);
}

/* Input residual, neighbors and prediction mode
 * overwrite reconstructed block on resudual
 */
void intra16x16_reconstruct(Block16x16& block,
  std::experimental::optional<std::reference_wrapper<Block16x16>> ul,
  std::experimental::optional<std::reference_wrapper<Block16x16>> u,
  std::experimental::optional<std::reference_wrapper<Block16x16>> l,
  const Intra16x16Mode mode) {

  Block16x16 pred;
  Predictor predictor = get_intra16x16_predictor(ul, u, l);
  get_intra16x16(pred, predictor, mode);

  std::transform(block.begin(), block.end(), pred.begin(), block.begin(), std::plus<int>());
}

/* Input predictors and mode
 * write intra16x16 prediction on pred
 */
void get_intra16x16(Block16x16& pred, const Predictor& p, const Intra16x16Mode mode) {
  switch (mode) {
    case Intra16x16Mode::VERTICAL:
      intra16x16_vertical(pred, p);
      break;
    case Intra16x16Mode::HORIZONTAL:
      intra16x16_horizontal(pred, p);
      break;
    case Intra16x16Mode::DC:
      intra16x16_dc(pred, p);
      break;
    case Intra16x16Mode::PLANE:
      intra16x16_plane(pred, p);
      break;
  }
}

void intra16x16_vertical(Block16x16& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  int i;
  for (i = 0; i < 16; i++) {
    std::copy_n(p.begin()+1, 16, pred.begin()+i*16);
  }
}

void intra16x16_horizontal(Block16x16& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  int i, j;
  for (i = 0; i < 16; i++) {
    for (j = 0; j < 16; j++) {
      pred[i*16+j] = p[17+i];
    }
  }
}

void intra16x16_dc(Block16x16& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  int s1 = 0, s2 = 0, s = 0;
  int i;

  for (i = 1; i < 17; i++) {
    s1 += p[i];
  }

  for (i = 17; i < 33; i++) {
    s2 += p[i];
  }

  if (predictor.up_available && predictor.left_available) {
    s = s1 + s2;
  }
  else if (!predictor.up_available && predictor.left_available) {
    s = 2 * s2;
  }
  else if (predictor.up_available && !predictor.left_available) {
    s = 2 * s1;
  }

  s += 16;
  s >>= 5;

  if (!predictor.up_available && !predictor.left_available) {
    s = 128;
  }

  pred.fill(s);
}

void intra16x16_plane(Block16x16& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  int H = 0, V = 0;
  int a, b, c;
  int i, j;

  for (i = 1; i < 8; i++) {
    H += i * (p[8+i] - p[8-i]);
    V += i * (p[24+i] - p[24-i]);
  }

  H += 8 * (p[16] - p[0]);
  V += 8 * (p[32] - p[0]);

  a = 16 * (p[16] + p[32]);
  b = (5 * H + 32) >> 6;
  c = (5 * V + 32) >> 6;

  for (i = 0; i < 16; i++) {
    for (j = 0; j < 16; j++) {
      pred[i*16+j] = clip((a + b * (j-7) + c * (i-7) + 16) >> 5, 0, 255);
    }
  }
}

/* Get intra16x16 predictors from neighbors
 * [0]: downmost and rightmost pixel of ul
 * [1..16]: downmost row of u
 * [17..32]: rightmost column of l
 */
Predictor get_intra16x16_predictor(
  std::experimental::optional<std::reference_wrapper<Block16x16>> ul,
  std::experimental::optional<std::reference_wrapper<Block16x16>> u,
  std::experimental::optional<std::reference_wrapper<Block16x16>> l) {

  Predictor predictor(16);
  std::vector<int>& p = predictor.pred_pel;
  // Check whether neighbors are available
  if (u) {
    Block16x16& tmp = *u;
    std::copy_n(tmp.begin()+16*15, 16, p.begin()+1);
    predictor.up_available = true;
  }
  else {
    std::fill_n(p.begin()+1, 16, 128);
  }

  if (l) {
    Block16x16& tmp = *l;
    for (int i = 0; i < 16; i++) {
      p[17+i] = tmp[i*16+15];
    }
    predictor.left_available = true;
  }
  else {
    std::fill_n(p.begin()+17, 16, 128);
  }

  if (predictor.up_available && predictor.left_available) {
    Block16x16& tmp = *ul;
    p[0] = tmp.back();
    predictor.all_available = true;
  }
  else {
    p[0] = 128;
  }

  return predictor;
}

/* Input 8x8 chroma block and its neighbors
 * do intra8x8 prediction which has 4 modes
 * overwrite residual on input block
 * return the least cost mode
 */
std::tuple<int, IntraChromaMode> intra8x8_chroma(Block8x8& cr_block,
  std::experimental::optional<std::reference_wrapper<Block8x8>> cr_ul,
  std::experimental::optional<std::reference_wrapper<Block8x8>> cr_u,
  std::experimental::optional<std::reference_wrapper<Block8x8>> cr_l,
  Block8x8& cb_block,
  std::experimental::optional<std::reference_wrapper<Block8x8>> cb_ul,
  std::experimental::optional<std::reference_wrapper<Block8x8>> cb_u,
  std::experimental::optional<std::reference_wrapper<Block8x8>> cb_l) {

  // Get Cr, Cb predictors
  Predictor cr_predictor = get_intra8x8_chroma_predictor(cr_ul, cr_u, cr_l);
  Predictor cb_predictor = get_intra8x8_chroma_predictor(cb_ul, cb_u, cb_l);

  int mode;
  IntraChromaMode best_mode;
  Block8x8 cr_pred, cb_pred, cr_residual, cb_residual;
  int min_sad = (1 << 15), cr_sad, cb_sad, sad;
  // Run all modes to get least residual
  for (mode = 0; mode < 4; mode++) {
    if ((!cr_predictor.up_available   && (IntraChromaMode::VERTICAL   == static_cast<IntraChromaMode>(mode))) ||
        (!cr_predictor.left_available && (IntraChromaMode::HORIZONTAL == static_cast<IntraChromaMode>(mode))) ||
        (!cr_predictor.all_available  && (IntraChromaMode::PLANE      == static_cast<IntraChromaMode>(mode)))) {
      continue;
    }

    get_intra8x8_chroma(cr_pred, cr_predictor, static_cast<IntraChromaMode>(mode));
    get_intra8x8_chroma(cb_pred, cb_predictor, static_cast<IntraChromaMode>(mode));

    cr_sad = SAD(cr_block.begin(), cr_block.end(), cr_pred.begin(), cr_pred.begin());
    cb_sad = SAD(cb_block.begin(), cb_block.end(), cb_pred.begin(), cb_pred.begin());
    sad = cr_sad + cb_sad;
    if (sad < min_sad) {
      min_sad = sad;
      best_mode = static_cast<IntraChromaMode>(mode);
      std::copy(cr_pred.begin(), cr_pred.end(), cr_residual.begin());
      std::copy(cb_pred.begin(), cb_pred.end(), cb_residual.begin());
    }
  }
  std::copy(cr_residual.begin(), cr_residual.end(), cr_block.begin());
  std::copy(cb_residual.begin(), cb_residual.end(), cb_block.begin());

  return std::make_tuple(min_sad, best_mode);
}

/* Input residual, neighbors and prediction mode
 * overwrite reconstructed block on resudual
 */
void intra8x8_chroma_reconstruct(Block8x8& block,
  std::experimental::optional<std::reference_wrapper<Block8x8>> ul,
  std::experimental::optional<std::reference_wrapper<Block8x8>> u,
  std::experimental::optional<std::reference_wrapper<Block8x8>> l,
  const IntraChromaMode mode) {

  Block8x8 pred;
  Predictor predictor = get_intra8x8_chroma_predictor(ul, u, l);
  get_intra8x8_chroma(pred, predictor, mode);

  std::transform(block.begin(), block.end(), pred.begin(), block.begin(), std::plus<int>());
}

/* Input predictors and mode
 * write intra8x8 chroma prediction on pred
 */
void get_intra8x8_chroma(Block8x8& pred, const Predictor& p, const IntraChromaMode mode) {
  switch (mode) {
    case IntraChromaMode::DC:
      intra8x8_chroma_dc(pred, p);
      break;
    case IntraChromaMode::HORIZONTAL:
      intra8x8_chroma_horizontal(pred, p);
      break;
    case IntraChromaMode::VERTICAL:
      intra8x8_chroma_vertical(pred, p);
      break;
    case IntraChromaMode::PLANE:
      intra8x8_chroma_plane(pred, p);
      break;
  }
}

void intra8x8_chroma_dc(Block8x8& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
  int s_upper_left = 0, s_upper_right = 0, s_down_left = 0, s_down_right = 0;
  int i, j;

  // summation of predictors
  // s1: [1..4], s2: [5..8]
  // s3: [9..12], s4: [13..16]
  for (i = 0; i < 4; i++) {
    s1 += p[i+1];
    s2 += p[i+5];
    s3 += p[i+9];
    s4 += p[i+13];
  }

  if (predictor.up_available && predictor.left_available) {
    s_upper_left = s1 + s3;
    s_upper_right = s2 + s3;
    s_down_left = s1 + s4;
    s_down_right = s2 + s4;
  }
  else if (!predictor.up_available && predictor.left_available) {
    s_upper_left = s_upper_right = 2 * s3;
    s_down_left = s_down_right = 2 * s4;
  }
  else if (predictor.up_available && !predictor.left_available) {
    s_upper_left = s_down_left = 2 * s1;
    s_upper_right = s_down_right = 2 * s2;
  }

  s_upper_left = (s_upper_left + 4) >> 3;
  s_upper_right = (s_upper_right + 4) >> 3;
  s_down_left = (s_down_left + 4) >> 3;
  s_down_right = (s_down_right + 4) >> 3;

  if (!predictor.up_available && !predictor.left_available) {
    s_upper_left = s_upper_right = s_down_left = s_down_right  = 128;
  }

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      pred[i*8+j] = s_upper_left;
      pred[i*8+(j+4)] = s_upper_right;
      pred[(i+4)*8+j] = s_down_left;
      pred[(i+4)*8+(j+4)] = s_down_right;
    }
  }
}

void intra8x8_chroma_horizontal(Block8x8& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  int i, j;
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      pred[i*8+j] = p[9+i];
    }
  }
}

void intra8x8_chroma_vertical(Block8x8& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  int i;
  for (i = 0; i < 8; i++) {
    std::copy_n(p.begin()+1, 8, pred.begin()+i*8);
  }
}

void intra8x8_chroma_plane(Block8x8& pred, const Predictor& predictor) {
  const std::vector<int>& p = predictor.pred_pel;
  int H = 0, V = 0;
  int a, b, c;
  int i, j;

  for (i = 1; i < 4; i++) {
    H += i * (p[4+i] - p[4-i]);
    V += i * (p[12+i] - p[12-i]);
  }

  H += 4 * (p[8] - p[0]);
  V += 4 * (p[16] - p[0]);

  a = 16 * (p[8] + p[16]);
  b = (17 * H + 16) >> 5;
  c = (17 * V + 16) >> 5;

  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      pred[i*8+j] = clip((a + b * (j-3) + c * (i-3) + 16) >> 5, 0, 255);
    }
  }
}

/* Get intra8x8 chroma predictors from neighbors
 * [0]: downmost and rightmost pixel of ul
 * [1..8]: downmost row of u
 * [9..16]: rightmost column of l
 */
Predictor get_intra8x8_chroma_predictor(
  std::experimental::optional<std::reference_wrapper<Block8x8>> ul,
  std::experimental::optional<std::reference_wrapper<Block8x8>> u,
  std::experimental::optional<std::reference_wrapper<Block8x8>> l) {

  Predictor predictor(8);
  std::vector<int>& p = predictor.pred_pel;
  // Check whether neighbors are available
  if (u) {
    Block8x8& tmp = *u;
    std::copy_n(tmp.begin()+8*7, 8, p.begin()+1);
    predictor.up_available = true;
  }
  else {
    std::fill_n(p.begin()+1, 8, 128);
  }

  if (l) {
    Block8x8& tmp = *l;
    for (int i = 0; i < 8; i++) {
      p[9+i] = tmp[i*8+7];
    }
    predictor.left_available = true;
  }
  else {
    std::fill_n(p.begin()+9, 8, 128);
  }

  if (predictor.up_available && predictor.left_available) {
    Block8x8& tmp = *ul;
    p[0] = tmp.back();
    predictor.all_available = true;
  }
  else {
    p[0] = 128;
  }

  return predictor;
}
