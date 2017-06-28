#ifndef BITSTREAM
#define BITSTREAM

#include <cassert>
#include <cstdint>
#include <vector>
#include <string>
#include <bitset>

class Bitstream {
public:
  int nb_bits;
  std::vector<std::uint8_t> buffer;

  Bitstream();
  Bitstream(const bool&);
  Bitstream(std::uint8_t[], int);
  Bitstream(const Bitstream&);
  Bitstream(const std::string&);
  Bitstream(const std::uint8_t, int);
  Bitstream(const unsigned int, int);

  Bitstream operator+(const Bitstream&);
  Bitstream& operator+=(const Bitstream&);

  bool byte_align();
  Bitstream rbsp_trailing_bits();
  Bitstream rbsp_to_ebsp();

  // for testing
  std::string to_string();
};

#endif // BITSTREAM

