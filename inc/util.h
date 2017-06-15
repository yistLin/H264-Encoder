#ifndef UTIL
#define UTIL

#include <iostream>
#include <map>

#include "log.h"

class Util {
public:
  unsigned int width, height;
  std::string input_file, output_file;

  Util(const int, const char*[]);

private:
  Log logger;
};

#endif
