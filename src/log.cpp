#include "log.h"

bool Log::log_normal = true;
bool Log::log_error = true;
bool Log::log_verbose = false;
bool Log::log_debug = false;

Log::Log() : Log::Log("Default") {}

Log::Log(std::string&& block_name) : block_name{"[" + block_name + "] "} {}

void Log::log(Level level, std::experimental::string_view message) {
  switch (level) {
    case Level::NORMAL:
      if (this->log_normal)
        std::cerr << this->block_name << message << std::endl;
      break;
    case Level::ERROR:
      if (this->log_error)
        std::cerr << this->block_name << message << std::endl;
      break;
    case Level::VERBOSE:
      if (this->log_verbose)
        std::cerr << this->block_name << message << std::endl;
      break;
    case Level::DEBUG:
      if (this->log_debug)
        std::cerr << this->block_name << message << std::endl;
      break;
  }
}
