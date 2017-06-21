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

/* Input 16x16 block and its neighbors
 * do intra16x16 prediction which has 4 modes
 * overwrite residual on input block
 * return the least cost mode
 */
Intra16x16Mode intra16x16(Block16x16& block, 
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
    get_intra16x16(pred, predictor, static_cast<Intra16x16Mode>(mode));

    sad = SAD(block.begin(), block.end(), pred.begin(), pred.begin());
    if (sad < min_sad) {
      min_sad = sad;
      best_mode = static_cast<Intra16x16Mode>(mode);
      std::copy(pred.begin(), pred.end(), residual.begin());
    }
  }
  std::copy(residual.begin(), residual.end(), block.begin());

  return best_mode;
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
    predictor.up_available = true;
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

IntraChromaMode intra8x8_chroma(Block8x8& block,
  std::experimental::optional<std::reference_wrapper<Block8x8>> ul,
  std::experimental::optional<std::reference_wrapper<Block8x8>> u,
  std::experimental::optional<std::reference_wrapper<Block8x8>> l) {

}