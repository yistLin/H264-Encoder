#include "bitstream.h"

Bitstream::Bitstream(): nb_bits(0) {}

Bitstream::Bitstream(std::uint8_t bits[], int digit) {
  nb_bits = digit;

  int nb_int = digit % 8 == 0 ? digit/8 : digit/8 + 1;
  for (int i = 0; i < nb_int; i++)
    buffer.push_back(bits[i]);
}

Bitstream::Bitstream(const Bitstream& a) {
  nb_bits = a.nb_bits;
  buffer.insert(buffer.end(), a.buffer.begin(), a.buffer.end());
}

Bitstream& Bitstream::operator+=(const Bitstream& a) {
  int trail_bits = this->nb_bits % 8;

  if (trail_bits == 0) {
    this->buffer.insert(this->buffer.end(), a.buffer.begin(), a.buffer.end());
  }
  else {
    std::uint8_t tmp = this->buffer.back();
    this->buffer.pop_back();

    // clear tmp
    tmp &= (0b11111111 << (8 - trail_bits));

    int last_bits = a.nb_bits;
    auto itr = a.buffer.begin();

    while (last_bits >= 8) {
      tmp |= *itr >> trail_bits;
      this->buffer.push_back(tmp);
      tmp = *itr << (8 - trail_bits);
      last_bits -= 8;
      itr++;
    }

    tmp |= *itr >> trail_bits;
    this->buffer.push_back(tmp);

    if (last_bits > (8 - trail_bits)) {
      tmp = *itr << (8 - trail_bits);
      this->buffer.push_back(tmp);
    }
  }

  this->nb_bits += a.nb_bits;
  return *this;
}

Bitstream Bitstream::operator+(const Bitstream& a) {
  Bitstream c(*this);
  int trail_bits = c.nb_bits % 8;

  if (trail_bits == 0) {
    c.buffer.insert(c.buffer.end(), a.buffer.begin(), a.buffer.end());
  }
  else {
    std::uint8_t tmp = c.buffer.back();
    c.buffer.pop_back();

    // clear tmp
    tmp &= (0b11111111 << (8 - trail_bits));

    int last_bits = a.nb_bits;
    auto itr = a.buffer.begin();

    while (last_bits >= 8) {
      tmp |= *itr >> trail_bits;
      c.buffer.push_back(tmp);
      tmp = *itr << (8 - trail_bits);
      last_bits -= 8;
      itr++;
    }

    tmp |= *itr >> trail_bits;
    c.buffer.push_back(tmp);

    if (last_bits > (8 - trail_bits)) {
      tmp = *itr << (8 - trail_bits);
      c.buffer.push_back(tmp);
    }
  }

  c.nb_bits += a.nb_bits;
  return c;
}

std::string Bitstream::to_string() {
  int nb_full_digit = nb_bits / 8;
  int trail_bits = nb_bits % 8;

  std::string s;
  for (int i = 0; i < nb_full_digit; i++) {
    std::bitset<8> bits(buffer[i]);
    s += bits.to_string() + " ";
  }

  if (trail_bits != 0) {
    std::bitset<8> bits(buffer[nb_full_digit]);
    s += bits.to_string().substr(0, trail_bits);
  }

  return s;
}

