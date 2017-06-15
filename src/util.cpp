#include "util.h"

Util::Util(const int argc, const char *argv[]) {
  this->logger = Log("Util");

  std::map<std::string, std::string> options{{"verbose", "false"},
                                             {"width", "0"},
                                             {"height", "0"},
                                             {"input_file", "snoopy.avi"},
                                             {"output_file", "snoopy.264"}};
  for (int i = 1; i < argc; i++) {
    std::string key, value{"true"};
    std::stringstream argument;
    argument.str(argv[i]);

    if (argument.peek() == '-') {
      argument.get();
      if (argument.peek() == '-') {
        argument.get();
        std::getline(argument, key, '=');
        std::getline(argument, value);
      } else {
        std::getline(argument, key);
      }
      options[key] = value;
    }
  }

  Log::log_verbose = options["verbose"] == "true";
  this->logger.log(Level::VERBOSE, "Setting width to " + options["width"]);
  this->width = std::stoul(options["width"]);
  this->logger.log(Level::VERBOSE, "Setting height to " + options["height"]);
  this->height = std::stoul(options["height"]);
  this->logger.log(Level::VERBOSE, "Setting input file to " + options["input_file"]);
  this->input_file = options["input_file"];
  this->logger.log(Level::VERBOSE, "Setting output file to " + options["output_file"]);
  this->output_file = options["output_file"];
}
