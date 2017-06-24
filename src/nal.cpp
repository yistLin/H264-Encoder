#include "nal.h"

/* Construct NAL unit 
 * given ref, type and string of data bits (SODB)
 */
NALUnit::NALUnit(const NALRefIdc ref_idc, const NALType type, const Bitstream& sodb) {
  forbidden_zero_bit = 0;
  nal_ref_idc = ref_idc;
  nal_unit_type = type;
  buffer = sodb;
}

/* Return NAL header 
 * which has 8 bits 
 *          1                2             5
 * |------------------|------------|--------------|
 * 
 *  forbidden_zero_bit  nal_ref_idc  nal_unit_type 
 */
std::uint8_t NALUnit::nal_header() {
  return (forbidden_zero_bit << 7) | (static_cast<uint8_t>(nal_ref_idc) << 5) |
           (static_cast<uint8_t>(nal_unit_type));
}