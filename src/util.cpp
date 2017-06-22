#include "util.h"

Util::Util(const int argc, const char *argv[]) {
  this->logger = Log("Util");

  std::map<std::string, std::string> options{{"v", "false"},
                                             {"d", "false"},
                                             {"size", "0x0"},
                                             {"input", "snoopy.avi"},
                                             {"output", "snoopy.264"},
                                             {"t", "-1"}};

  // get arguments from command line
  std::string key;
  for (int i = 1; i < argc; i++) {

    if (key.empty()) {
      std::istringstream argument{argv[i]};
      if (argument.peek() == '-') {
        argument.get();
        if (argument.peek() == '-') {
          argument.get();
          std::getline(argument, key);
        } else {
          std::getline(argument, key);
          options[key] = "true";
          key = "";
        }
      }
    } else {
      options[key] = argv[i];
      key = "";
    }
  }

  Log::log_verbose = options["v"] == "true";
  Log::log_debug = options["d"] == "true";

  // parse size to width and height
  std::istringstream size{options["size"]};
  std::string width_s, height_s;
  std::getline(size, width_s, 'x');
  std::getline(size, height_s);
  if (width_s.empty())
    width_s = "0";
  if (height_s.empty())
    height_s = "0";
  this->width = std::stoul(width_s);
  this->logger.log(Level::VERBOSE, "Setting width to " + std::to_string(this->width));
  this->height = std::stoul(height_s);
  this->logger.log(Level::VERBOSE, "Setting height to " + std::to_string(this->height));

  // parse input and output file path
  this->input_file = options["input"];
  this->logger.log(Level::VERBOSE, "Setting input file to " + this->input_file);
  this->output_file = options["output"];
  this->logger.log(Level::VERBOSE, "Setting output file to " + this->output_file);

  this->test_frame = std::stoul(options["t"]);
}
