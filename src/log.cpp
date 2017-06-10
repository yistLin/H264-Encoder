#include "log.h"

Log::Log() {}

void Log::log(Level level, std::string message) {
  std::cerr << message << std::endl;
}
