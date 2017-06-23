#include "vlc.h"

/* Unsigned Exponential Golomb coding
 */
Bitstream exp_golomb(const int codenum) {
  int x, leading_zeros, nb_bits;
  x = codenum + 1;
  leading_zeros = static_cast<int> (log2(x));
  nb_bits = (leading_zeros << 1) + 1;

  std::string codeword = std::bitset<64>(x).to_string();
  codeword = codeword.substr(64-nb_bits, nb_bits);
  return Bitstream(codeword);
}

/* Signed Exponential Golomb coding
 */
Bitstream signed_exp_golomb(const int codenum) {
  int _codenum = (codenum > 0) ? 2*codenum-1 : 2*(-codenum);
  return exp_golomb(_codenum);
}