#ifndef LOG
#define LOG

#include <iostream>
#include <string>

enum class Level { ERROR, VERBOSE, DEBUG };

class Log {
public:
  Log();
  void log(Level, std::string);
};

#endif
