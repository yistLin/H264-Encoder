#ifndef BLOCK
#define BLOCK

#include <array>
#include <functional>

#define PIXELS_PER_BLOCK 8*8

class Block2x2 {
private:
  std::array<std::reference_wrapper<int>, 4> data;

public:
  Block2x2(int& e1, int& e2, int &e3, int &e4) : data{{e1, e2, e3, e4}} {}

  int& operator[](int index) { return data[index].get(); }
  decltype(std::begin(data)) begin() { return std::begin(data); }
  decltype(std::end(data)) end() { return std::end(data); }
};

class Block4x4 {
private:
  std::array<std::reference_wrapper<int>, 16> data;

public:
  Block4x4(int& e1, int& e2, int &e3, int &e4, int &e5, int &e6, int &e7, int &e8, int &e9, int &e10, int &e11, int &e12, int &e13, int &e14, int &e15, int &e16) : data{{e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16}} {};

  int& operator[](int index) { return data[index].get(); }
  decltype(std::begin(data)) begin() { return std::begin(data); }
  decltype(std::end(data)) end() { return std::end(data); }
};

using Block8x8 = std::array<int, 8*8>;
using Block16x16 = std::array<int, 16*16>;

#endif
