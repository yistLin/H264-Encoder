#include <iostream>

#include "log.h"
#include "util.h"
#include "io.h"
#include "frame.h"
#include "frame_encode.h"
#include "frame_vlc.h"

Log logger("Main");

void* operator new(std::size_t n) {
    // std::cerr << "[allocating " << n << " bytes]\n";
    return malloc(n);
}

void encode_sequence(Reader& reader, Writer& writer, Util& util) {
  int curr_frame = 0;

  writer.write_sps(util.width, util.height, reader.nb_frames);
  writer.write_pps();

  while (curr_frame < reader.nb_frames) {
    Frame frame(reader.get_padded_frame());

    logger.log(Level::NORMAL, "encode frame #" + std::to_string(curr_frame));
    if (util.test_frame != -1) {
      if (curr_frame == util.test_frame) {
        encode_I_frame(frame);
        break;
      }
    } else {
      encode_I_frame(frame);
      vlc_frame(frame, writer);
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
