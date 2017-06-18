#include <iostream>

#include "log.h"
#include "util.h"
#include "io.h"
#include "frame.h"

void encode_sequence(Reader& reader) {
  int curr_frame = 0;

  while (curr_frame < reader.nb_frames) {
    PadFrame pf = reader.get_padded_frame();
    Frame frame(pf);

    curr_frame++;
  }
}

int main(int argc, const char *argv[]) {
  Log logger("Main");

  // Get command-line arguments
  Util util(argc, argv);

  // Read from given filename
  Reader reader(util.input_file, util.width, util.height);

  // Encoding process start
  encode_sequence(reader);

  return EXIT_SUCCESS;
}

