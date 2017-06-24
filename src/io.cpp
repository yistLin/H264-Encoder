#include "io.h"

RawFrame::RawFrame(const int w, const int h): width(w), height(h) {}

PadFrame::PadFrame(const int w, const int h) {
  this->raw_width = w;
  this->raw_height = h;

  // Padding width and height to multiple of 16
  int pd;
  this->width = ((pd = w % 16) > 0) ? w + 16 - pd : w;
  this->height = ((pd = h % 16) > 0) ? h + 16 - pd : h;
}

PadFrame::PadFrame(const RawFrame& rf) {
  this->raw_width = rf.width;
  this->raw_height = rf.height;

  // Padding width and height to multiple of 16
  int pd;
  this->width = ((pd = rf.width % 16) > 0) ? rf.width + 16 - pd : rf.width;
  this->height = ((pd = rf.height % 16) > 0) ? rf.height + 16 - pd : rf.height;

  // Reserve the pixel vectors
  int pixels_per_unit = this->width * this->height;
  this->Y.reserve(pixels_per_unit);
  this->Cr.reserve(pixels_per_unit);
  this->Cb.reserve(pixels_per_unit);

  // Set up pointers for insertion
  auto y_ptr = rf.Y.begin();
  auto cr_ptr = rf.Cr.begin();
  auto cb_ptr = rf.Cb.begin();
  auto pad_y_ptr = this->Y.begin();
  auto pad_cr_ptr = this->Cr.begin();
  auto pad_cb_ptr = this->Cb.begin();

  // Insert into vectors
  for (int i = 0; i < rf.height; i++) {
    // Insert original pixels
    this->Y.insert(pad_y_ptr, y_ptr, y_ptr + rf.width);
    this->Cr.insert(pad_cr_ptr, cr_ptr, cr_ptr + rf.width);
    this->Cb.insert(pad_cb_ptr, cb_ptr, cb_ptr + rf.width);

    // Insert padding pixels (additional width)
    this->Y.insert(pad_y_ptr + rf.width, this->width - rf.width, 0);
    this->Cr.insert(pad_cr_ptr + rf.width, this->width - rf.width, 0);
    this->Cb.insert(pad_cb_ptr + rf.width, this->width - rf.width, 0);

    // Move pointers
    y_ptr += rf.width;
    cr_ptr += rf.width;
    cb_ptr += rf.width;
    pad_y_ptr += this->width;
    pad_cb_ptr += this->width;
    pad_cr_ptr += this->width;
  }

  // Insert padding pixels (additional height)
  for (int i = rf.height; i < this->height; i++) {
    this->Y.insert(pad_y_ptr, this->width, 0);
    this->Cr.insert(pad_cr_ptr, this->width, 0);
    this->Cb.insert(pad_cb_ptr, this->width, 0);

    // Move pointers
    pad_y_ptr += this->width;
    pad_cb_ptr += this->width;
    pad_cr_ptr += this->width;
  }
}

Reader::Reader(std::string filename, const int wid, const int hei) {
  this->width = wid;
  this->height = hei;

  // Open the file stream for raw video file
  this->file.open(filename, std::ios::in | std::ios::binary);
  if (!this->file.is_open()) {
    this->logger.log(Level::ERROR, "Cannot open file");
    exit(1);
  }

  // Get file size
  this->file_size = this->get_file_size();

  // Calculate number of frames
  this->pixels_per_unit = wid * hei;
  this->pixels_per_frame = this->pixels_per_unit * 3;
  this->nb_frames = this->file_size / this->pixels_per_frame;

  // Initialize logging tool
  this->logger = Log("Reader");
  this->logger.log(Level::VERBOSE, "file size = " + std::to_string(this->file_size));
  this->logger.log(Level::VERBOSE, "# of frames = " + std::to_string(this->nb_frames));
}

std::size_t Reader::get_file_size() {
  // Get file size
  std::size_t begin_pos = this->file.tellg();
  this->file.seekg(0, std::ios::end);
  std::size_t end_pos = this->file.tellg();

  // Go back to the beginning of file
  this->file.clear();
  this->file.seekg(0, std::ios::beg);

  return end_pos - begin_pos;
}

void Reader::convert_rgb_to_ycrcb(unsigned char* rgb_pixel, double& y, double& cr, double& cb) {
  double b, g, r;

  // Convert from char to RGB value
  r = (double)rgb_pixel[0];
  g = (double)rgb_pixel[1];
  b = (double)rgb_pixel[2];

  // Convert from RGB to YCrCb
  y  = std::round( 0.257 * r + 0.504 * g + 0.098 * b + 16);
  cb = std::round(-0.148 * r - 0.291 * g + 0.439 * b + 128);
  cr = std::round( 0.439 * r - 0.368 * g - 0.071 * b + 128);
}

RawFrame Reader::read_one_frame() {
  unsigned char rgb_pixel[3];
  double y, cb, cr;

  // Reserve frame-sized bytes
  RawFrame rf(this->width, this->height);
  rf.Y.reserve(this->pixels_per_unit);
  rf.Cb.reserve(this->pixels_per_unit);
  rf.Cr.reserve(this->pixels_per_unit);

  for (int i = 0; i < this->pixels_per_unit; i++) {
    // Read 3 bytes as 1 pixel
    this->file.read((char*)rgb_pixel, 3);

    this->convert_rgb_to_ycrcb(rgb_pixel, y, cr, cb);

    // Fill to pixel array
    rf.Y[i] = y;
    rf.Cb[i] = cb;
    rf.Cr[i] = cr;
  }

  return rf;
}

PadFrame Reader::get_padded_frame() {
  unsigned char rgb_pixel[3];
  double y, cb, cr;
  PadFrame pf(this->width, this->height);

  // Reserve the pixel vectors
  int pixels_per_unit = pf.width * pf.height;
  pf.Y.resize(pixels_per_unit);
  pf.Cr.resize(pixels_per_unit);
  pf.Cb.resize(pixels_per_unit);

  std::fill(pf.Y.begin(), pf.Y.end(), 0);
  std::fill(pf.Cr.begin(), pf.Cr.end(), 128);
  std::fill(pf.Cb.begin(), pf.Cb.end(), 128);

  for (int i = 0; i < pf.raw_height; i++) {
    for (int j = 0; j < pf.raw_width; j++) {
      // Read 3 bytes as 1 pixel
      this->file.read((char*)rgb_pixel, 3);

      this->convert_rgb_to_ycrcb(rgb_pixel, y, cr, cb);

      // Fill to pixel array
      pf.Y[i*pf.width+j] = y;
      pf.Cb[i*pf.width+j] = cb;
      pf.Cr[i*pf.width+j] = cr;
    }
  }

  return pf;
}

Writer::Writer(std::string filename) {
  // Open the file stream for output file
  file.open(filename, std::ios::out | std::ios::binary);
  if (!file.is_open()) {
    logger.log(Level::ERROR, "Cannot open file");
    exit(1);
  }
}

std::uint8_t Writer::stopcode[4] = {0x00, 0x00, 0x00, 0x01};

void Writer::write_sps(const int width, const int height, const int num_frames) {
  Bitstream output(stopcode, 32);
  Bitstream rbsp = seq_parameter_set_rbsp(width, height, num_frames);
  NALUnit nal_unit(NALRefIdc::HIGHEST, NALType::SPS, rbsp);

  output += nal_unit.get();

  file.write((char*)&output.buffer[0], output.buffer.size());
  file.close();
}

void Writer::write_pps() {
  Bitstream output(stopcode, 32);
  Bitstream rbsp = pic_parameter_set_rbsp();
  
  NALUnit nal_unit(NALRefIdc::HIGHEST, NALType::PPS, rbsp);

  output += nal_unit.get();
  file.write((char*)&output.buffer[0], output.buffer.size());
}

void Writer::write_slice(const Bitstream& slice_data) {
  Bitstream output(stopcode, 32);
  Bitstream rbsp = slice_layer_without_partitioning_rbsp(slice_data);

  NALUnit nal_unit(NALRefIdc::HIGHEST, NALType::IDR, rbsp);

  output += nal_unit.get();
  file.write((char*)&output.buffer[0], output.buffer.size());
}

Bitstream Writer::seq_parameter_set_rbsp(const int width, const int height, const int num_frames) {
  Bitstream sodb;
  // only support baseline profile
  std::uint8_t profile_idc = 66;
  bool constraint_set0_flag = false;
  bool constraint_set1_flag = false;
  bool constraint_set2_flag = false;
  std::uint8_t reserved_zero_5bits = 0x00;
  std::uint8_t level_idc = 10;
  unsigned int seq_parameter_set_id = 0;
  unsigned int log2_max_frame_num_minus4 = static_cast<unsigned int>(log2(num_frames) - 4);
  unsigned int pic_order_cnt_type = 0;
  unsigned int log2_max_pic_order_cnt_lsb_minus4 = 0;
  unsigned int num_ref_frames = 0;
  bool gaps_in_frame_num_value_allowed_flag = false;
  unsigned int pic_width_in_mbs_minus_1 = (width % 16 == 0)? (width / 16) - 1 : width / 16;
  unsigned int pic_height_in_mbs_minus_1 = (height % 16 == 0)? (height / 16) - 1 : height / 16;
  bool frame_mbs_only_flag = true;
  bool direct_8x8_inference_flag = false;
  bool frame_cropping_flag = false;
  bool vui_parameters_present_flag = false;

  sodb = Bitstream(profile_idc, 8) + Bitstream(constraint_set0_flag) + 
         Bitstream(constraint_set1_flag) + Bitstream(constraint_set2_flag) + 
         Bitstream(reserved_zero_5bits, 5) + Bitstream(level_idc, 8) + 
         ue(seq_parameter_set_id) + ue(log2_max_frame_num_minus4) + 
         ue(pic_order_cnt_type) + ue(log2_max_pic_order_cnt_lsb_minus4) +
         ue(num_ref_frames) + Bitstream(gaps_in_frame_num_value_allowed_flag) +
         ue(pic_width_in_mbs_minus_1) + ue(pic_height_in_mbs_minus_1) + 
         Bitstream(frame_mbs_only_flag) + Bitstream(direct_8x8_inference_flag) +
         Bitstream(frame_cropping_flag) + Bitstream(vui_parameters_present_flag);

  return sodb.rbsp_trailing_bits();
}

Bitstream Writer::pic_parameter_set_rbsp() {
  return Bitstream();
}

Bitstream Writer::slice_layer_without_partitioning_rbsp(const Bitstream& slice_data) {
  return Bitstream() + slice_data;
}