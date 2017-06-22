#ifndef UTIL
#define UTIL

#include <sstream>
#include <string>
#include <map>

#include "log.h"

class Util {
public:
  unsigned int width, height;
  int test_frame;
  std::string input_file, output_file;

  Util(const int, const char*[]);

private:
  Log logger;
};

#endif
