#ifndef LOG
#define LOG

#include <iostream>
#include <string>

enum class Level { ERROR, VERBOSE, DEBUG };

class Log {
public:
  static void log(Level level, std::string message) {
    std::cerr << message << std::endl;
  }
};

#endif
