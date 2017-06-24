#ifndef NAL
#define NAL

#include <string>
#include <cstdint>

#include "bitstream.h"
#include "vlc.h"

/* For nal_unit_type
 * 2, 3, 4 do not appear in baseline profile
 */
enum class NALType {
 SLICE    = 1,
 DPA      = 2,
 DPB      = 3,
 DPC      = 4,
 IDR      = 5,
 SEI      = 6,
 SPS      = 7,
 PPS      = 8,
 AUD      = 9,
 EOSEQ    = 10,
 EOSTREAM = 11,
 FILL     = 12
};

/* For nal_ref_idc
 * the priority of NAL unit
 */
enum class NALRefIdc {
 HIGHEST     = 3,
 HIGH        = 2,
 LOW         = 1,
 DISPOSABLE  = 0
};

class NALUnit {
public:
  Bitstream buffer;

  NALUnit(const NALRefIdc, const NALType, const Bitstream&);
  std::uint8_t nal_header();
  Bitstream get();

private:
  int forbidden_zero_bit; // Always be zero
  NALRefIdc nal_ref_idc;
  NALType nal_unit_type;
};

#endif