#include <iostream>

#include "log.h"
#include "util.h"
#include "io.h"
#include "frame.h"
#include "frame_encode.h"

Log logger("Main");

void* operator new(std::size_t n) {
    // std::cerr << "[allocating " << n << " bytes]\n";
    return malloc(n);
}

void encode_sequence(Reader& reader, Writer& Writer, Util& util) {
  int curr_frame = 0;
  while (curr_frame < reader.nb_frames) {
    Frame frame(reader.get_padded_frame());

    logger.log(Level::VERBOSE, "encode frame #" + std::to_string(curr_frame));
    if (util.test_frame != -1) {
      if (curr_frame == util.test_frame) {
        encode_I_frame(frame);
        break;
      }
    } else {
      encode_I_frame(frame);
    }

    curr_frame++;
  }
}

int main(int argc, const char *argv[]) {
  // Get command-line arguments
  Util util(argc, argv);

  // Read from given filename
  Reader reader(util.input_file, util.width, util.height);

  // Write to given filename
  Writer writer(util.output_file);

  // Encoding process start
  encode_sequence(reader, writer, util);

  return EXIT_SUCCESS;
}
