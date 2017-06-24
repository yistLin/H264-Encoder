#include "io.h"

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
  std::uint8_t profile_idc = 66;  // u(8)
  bool constraint_set0_flag = false;  // u(1)
  bool constraint_set1_flag = false;  // u(1)
  bool constraint_set2_flag = false;  // u(1)
  std::uint8_t reserved_zero_5bits = 0x00;  // u(5)
  std::uint8_t level_idc = 10;  // u(8)
  unsigned int seq_parameter_set_id = 0;  // ue(v)
  unsigned int log2_max_frame_num_minus4 = static_cast<unsigned int>(log2(num_frames) - 4); // ue(v)
  unsigned int pic_order_cnt_type = 0;  // ue(v)
  unsigned int log2_max_pic_order_cnt_lsb_minus4 = 0; // ue(v)
  unsigned int num_ref_frames = 0;  // ue(v)
  bool gaps_in_frame_num_value_allowed_flag = false;  // u(1)
  unsigned int pic_width_in_mbs_minus_1 = (width % 16 == 0)? (width / 16) - 1 : width / 16; // ue(v)
  unsigned int pic_height_in_mbs_minus_1 = (height % 16 == 0)? (height / 16) - 1 : height / 16; // ue(v)
  bool frame_mbs_only_flag = true;  // u(1)
  bool direct_8x8_inference_flag = false; // u(1)
  bool frame_cropping_flag = false; // u(1)
  bool vui_parameters_present_flag = false; // u(1)

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
  Bitstream sodb;

  unsigned int pic_parameter_set_id = 0;  // ue(v)
  unsigned int seq_parameter_set_id = 0;  // ue(v)
  bool entropy_coding_mode_flag = false;  // u(1)
  bool pic_order_present_flag = false;  // u(1)
  unsigned int num_slice_groups_minus1 = 0; // ue(v)
  unsigned int num_ref_idx_l0_active_minus1 = 0;  // ue(v)
  unsigned int num_ref_idx_l1_active_minus1 = 0;  // ue(v)
  bool weighted_pred_flag = false;  // u(1)
  unsigned int weighted_bipred_idc = 0; // u(2)
  int pic_init_qp_minus26 = LUMA_QP - 26; // se(v)
  int pic_init_qs_minus26 = 0;  // se(v)
  int chroma_qp_index_offset = CHROMA_QP - LUMA_QP; // se(v)
  bool deblocking_filter_control_present_flag = true; // u(1)
  bool constrained_intra_pred_flag = false; // u(1)
  bool redundant_pic_cnt_present_flag = false;  // u(1)

  sodb = ue(pic_parameter_set_id) + ue(seq_parameter_set_id) +
         Bitstream(entropy_coding_mode_flag) + Bitstream(pic_order_present_flag) +
         ue(num_slice_groups_minus1) + ue(num_ref_idx_l0_active_minus1) +
         ue(num_ref_idx_l1_active_minus1) + Bitstream(weighted_pred_flag) +
         Bitstream(weighted_bipred_idc, 2) + se(pic_init_qp_minus26) +
         se(pic_init_qs_minus26) + se(chroma_qp_index_offset) +
         Bitstream(deblocking_filter_control_present_flag) + Bitstream(constrained_intra_pred_flag) +
         Bitstream(redundant_pic_cnt_present_flag);

  return sodb.rbsp_trailing_bits();
}

Bitstream Writer::slice_layer_without_partitioning_rbsp(const Bitstream& slice_data) {
  return Bitstream() + slice_data;
}

Bitstream Writer::write_slice_data(Frame& frame) {
  Bitstream sodb;

  for (auto& mb : frame.mbs) {
    if (mb.is_I_PCM) {
      sodb += ue(25);

      continue;
    }

    if (mb.is_intra16x16) {
      unsigned int type = 1;
      if (mb.coded_block_pattern_luma)
        type += 12;

      if (mb.coded_block_pattern_chroma_DC == false && mb.coded_block_pattern_chroma_AC)
        type += 0;
      else if (mb.coded_block_pattern_chroma_AC == false)
        type += 4;
      else
        type += 8;

      type += static_cast<unsigned int>(mb.intra16x16_Y_mode);

      sodb += ue(type);
    } else {
      sodb += ue(0);
    }

    sodb += mb_pred(mb);

    if (!mb.is_intra16x16) {
      unsigned int cbp = 0;
      if (mb.coded_block_pattern_chroma_DC == false && mb.coded_block_pattern_chroma_AC)
        cbp += 0;
      else if (mb.coded_block_pattern_chroma_AC == false)
        cbp += 16;
      else
        cbp += 32;

      if (mb.coded_block_pattern_luma)
        cbp += 15;

      sodb += me(cbp);
    }

    if (mb.coded_block_pattern_luma || mb.coded_block_pattern_chroma_DC || mb.coded_block_pattern_chroma_AC || mb.is_intra16x16) {
      sodb += se(0);
      sodb += mb.bitstream;
    }
  }

  return sodb;
}

Bitstream Writer::mb_pred(MacroBlock& mb) {
  Bitstream sodb;

  if (!mb.is_intra16x16) {
    unsigned int last = 0;
    for (auto& p_mode : mb.intra4x4_Y_mode) {
      unsigned int mode = static_cast<unsigned int>(p_mode);
      if (mode == last) {
        sodb += Bitstream(false);
      } else {
        sodb += Bitstream(true);
        if (mode < last)
          sodb += Bitstream(static_cast<std::uint8_t>(mode), 3);
        else
          sodb += Bitstream(static_cast<std::uint8_t>(mode - 1), 3);
        last = mode;
      }
    }
  }

  sodb += ue(static_cast<unsigned int>(mb.intra_Cr_Cb_mode));

  return sodb;
}
