#include "vlc.h"

Bitstream exp_golomb(const int codenum) {
  int x, leading_zeros, nb_bits;
  x = codenum + 1;
  leading_zeros = static_cast<int> (log2(x));
  nb_bits = (leading_zeros << 1) + 1;

  std::string codeword = std::bitset<64>(x).to_string();
  codeword = codeword.substr(64-nb_bits, nb_bits);
  return Bitstream(codeword);
}