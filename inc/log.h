#ifndef LOG
#define LOG

#include <iostream>
#include <string>
#include <experimental/string_view>

enum class Level { ERROR, VERBOSE, DEBUG };

class Log {
public:
  static bool log_error;
  static bool log_verbose;
  static bool log_debug;

  std::string block_name;

  Log();
  Log(std::string&&);
  void log(Level, std::experimental::string_view);
};

#endif
