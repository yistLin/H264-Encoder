#include "util.h"

Util::Util(const int argc, const char *argv[]) {
  this->logger = Log("Util");

  std::map<std::string, std::string> options{{"verbose", "false"},
                                             {"width", "0"},
                                             {"height", "0"},
                                             {"input_file", "snoopy.avi"},
                                             {"output_file", "snoopy.264"}};
  for (int i = 1; i < argc; i++) {
    std::string argument(argv[i]);

    if (argument.size() < 2)
      continue;

    if (argument.front() == '-') {
      if (argument[1] == '-') {
        auto pos = argument.find('=');
        auto key = argument.substr(2, pos - 2);
        if (pos == std::string::npos) {
          options[key] = "";
        } else {
          auto value = argument.substr(pos + 1);
          options[key] = value;
        }
      } else {
        auto key = argument.substr(1);
        options[key] = "true";
      }
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
