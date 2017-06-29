#include "vlc.h"
#include <iostream>

/* Zig-zag scan
 */
const int mat_zigzag4x4[16] = {
  0,  1,  5,  6,
  2,  4,  7, 12,
  3,  8, 11, 13,
  9, 10, 14, 15
};

/* Num-VLC table
 *
 * look-up table for "coeff_token" encoding
 *   num_vlc_table[ TableType ][ TotalCoeff ][ T1 ]
 */
std::string num_vlc_table[6][17][4] = {
  { // Num-VLC0
    { "1", "", "", "" },
    { "000101", "01", "", "" },
    { "00000111", "000100", "001", "" },
    { "000000111", "00000110", "0000101", "00011" },
    { "0000000111", "000000110", "00000101", "000011" },
    { "00000000111", "0000000110", "000000101", "0000100" },
    { "0000000001111", "00000000110", "0000000101", "00000100" },
    { "0000000001011", "0000000001110", "00000000101", "000000100" },
    { "0000000001000", "0000000001010", "0000000001101", "0000000100" },
    { "00000000001111", "00000000001110", "0000000001001", "00000000100" },
    { "00000000001011", "00000000001010", "00000000001101", "0000000001100" },
    { "000000000001111", "000000000001110", "00000000001001", "00000000001100" },
    { "000000000001011", "000000000001010", "000000000001101", "00000000001000" },
    { "0000000000001111", "000000000000001", "000000000001001", "000000000001100" },
    { "0000000000001011", "0000000000001110", "0000000000001101", "000000000001000" },
    { "0000000000000111", "0000000000001010", "0000000000001001", "0000000000001100" },
    { "0000000000000100", "0000000000000110", "0000000000000101", "0000000000001000" }
  },
  { // Num-VLC1
    { "11", "", "", "" },
    { "001011", "10", "", "" },
    { "000111", "00111", "011", "" },
    { "0000111", "001010", "001001", "0101" },
    { "00000111", "000110", "000101", "0100" },
    { "00000100", "0000110", "0000101", "00110" },
    { "000000111", "00000110", "00000101", "001000" },
    { "00000001111", "000000110", "000000101", "000100" },
    { "00000001011", "00000001110", "00000001101", "0000100" },
    { "000000001111", "00000001010", "00000001001", "000000100" },
    { "000000001011", "000000001110", "000000001101", "00000001100" },
    { "000000001000", "000000001010", "000000001001", "00000001000" },
    { "0000000001111", "0000000001110", "0000000001101", "000000001100" },
    { "0000000001011", "0000000001010", "0000000001001", "0000000001100" },
    { "0000000000111", "00000000001011", "0000000000110", "0000000001000" },
    { "00000000001001", "00000000001000", "00000000001010", "0000000000001" },
    { "00000000000111", "00000000000110", "00000000000101", "00000000000100" }
  },
  { // Num-VLC2
    { "1111", "", "", "" },
    { "001111", "1110", "", "" },
    { "001011", "01111", "1101", "" },
    { "001000", "01100", "01110", "1100" },
    { "0001111", "01010", "01011", "1011" },
    { "0001011", "01000", "01001", "1010" },
    { "0001001", "001110", "001101", "1001" },
    { "0001000", "001010", "001001", "1000" },
    { "00001111", "0001110", "0001101", "01101" },
    { "00001011", "00001110", "0001010", "001100" },
    { "000001111", "00001010", "00001101", "0001100" },
    { "000001011", "000001110", "00001001", "00001100" },
    { "000001000", "000001010", "000001101", "00001000" },
    { "0000001101", "000000111", "000001001", "000001100" },
    { "0000001001", "0000001100", "0000001011", "0000001010" },
    { "0000000101", "0000001000", "0000000111", "0000000110" },
    { "0000000001", "0000000100", "0000000011", "0000000010" }
  },
  { // FLC
    { "000011", "", "", "" },
    { "000000", "000001", "", "" },
    { "000100", "000101", "000110", "" },
    { "001000", "001001", "001010", "001011" },
    { "001100", "001101", "001110", "001111" },
    { "010000", "010001", "010010", "010011" },
    { "010100", "010101", "010110", "010111" },
    { "011000", "011001", "011010", "011011" },
    { "011100", "011101", "011110", "011111" },
    { "100000", "100001", "100010", "100011" },
    { "100100", "100101", "100110", "100111" },
    { "101000", "101001", "101010", "101011" },
    { "101100", "101101", "101110", "101111" },
    { "110000", "110001", "110010", "110011" },
    { "110100", "110101", "110110", "110111" },
    { "111000", "111001", "111010", "111011" },
    { "111100", "111101", "111110", "111111" }
  },
  { // For Nc = -1
    { "01", "", "", "" },
    { "000111", "1", "", "" },
    { "000100", "000110", "001", "" },
    { "000011", "0000011", "0000010", "000101" },
    { "000010", "00000011", "00000010", "0000000" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" }
  },
  { // For others
    { "1", "", "", "" },
    { "0001111", "01", "", "" },
    { "0001110", "0001101", "001", "" },
    { "000000111", "0001100", "0001011", "00001" },
    { "000000110", "000000101", "0001010", "000001" },
    { "0000000111", "0000000110", "000000100", "0001001" },
    { "00000000111", "00000000110", "0000000101", "0001000" },
    { "000000000111", "000000000110", "00000000101", "0000000100" },
    { "0000000000111", "000000000101", "000000000100", "00000000100" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" },
    { "", "", "", "" }
  }
};

/* Zero-TotalCoeff table
 *
 * used to encode total zeros
 *   zero_vlc_table[ TotalZeros ][ TotalCoeff ]
 */
std::string zero_vlc_table[16][17] = {
  { "", "1", "111", "0101", "00011", "0101", "000001", "000001", "000001", "000001", "00001", "0000", "0000", "000", "00", "0" },
  { "", "011", "110", "111", "111", "0100", "00001", "00001", "0001", "000000", "00000", "0001", "0001", "001", "01", "1", "" },
  { "", "010", "101", "110", "0101", "0011", "111", "101", "00001", "0001", "001", "001", "01", "1", "1", "", "" },
  { "", "0011", "100", "101", "0100", "111", "110", "100", "011", "11", "11", "010", "1", "01", "", "", "" },
  { "", "0010", "011", "0100", "110", "110", "101", "011", "11", "10", "10", "1", "001", "", "", "", "" },
  { "", "00011", "0101", "0011", "101", "101", "100", "11", "10", "001", "01", "011", "", "", "", "", "" },
  { "", "00010", "0100", "100", "100", "100", "011", "010", "010", "01", "0001", "", "", "", "", "", "" },
  { "", "000011", "0011", "011", "0011", "011", "010", "0001", "001", "00001", "", "", "", "", "", "", "" },
  { "", "000010", "0010", "0010", "011", "0010", "0001", "001", "000000", "", "", "", "", "", "", "", "" },
  { "", "0000011", "00011", "00011", "0010", "00001", "001", "000000", "", "", "", "", "", "", "", "", "" },
  { "", "0000010", "00010", "00010", "00010", "0001", "000000", "", "", "", "", "", "", "", "", "", "" },
  { "", "00000011", "000011", "000001", "00001", "00000", "", "", "", "", "", "", "", "", "", "", "" },
  { "", "00000010", "000010", "00001", "00000", "", "", "", "", "", "", "", "", "", "", "", "" },
  { "", "000000011", "000001", "000000", "", "", "", "", "", "", "", "", "", "", "", "", "" },
  { "", "000000010", "000000", "", "", "", "", "", "", "", "", "", "", "", "", "", "" },
  { "", "000000001", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "" }
};

/* Zero-TotalCoeff table for Chroma DC 2x2
 *
 *   zero_vlc_table2x2[ TotalZeros ][ TotalCoeff ]
 */
std::string zero_vlc_table2x2[4][4] = {
  { "", "1", "1", "1" },
  { "", "01", "01", "0" },
  { "", "001", "00", "" },
  { "", "000", "", "" }
};

/* Run-Length table
 *
 * used to encoding run-length of zeros
 *   run_vlc_table[ RunBefore ][ ZerosLeft ]
 */
std::string run_vlc_table[15][8] = {
  { "", "1", "1", "11", "11", "11", "11", "111" },
  { "", "0", "01", "10", "10", "10", "000", "110" },
  { "", "", "00", "01", "01", "011", "001", "101" },
  { "", "", "", "00", "001", "010", "011", "100" },
  { "", "", "", "", "000", "001", "010", "011" },
  { "", "", "", "", "", "000", "101", "010" },
  { "", "", "", "", "", "", "100", "001" },
  { "", "", "", "", "", "", "", "0001" },
  { "", "", "", "", "", "", "", "00001" },
  { "", "", "", "", "", "", "", "000001" },
  { "", "", "", "", "", "", "", "0000001" },
  { "", "", "", "", "", "", "", "00000001" },
  { "", "", "", "", "", "", "", "000000001" },
  { "", "", "", "", "", "", "", "0000000001" },
  { "", "", "", "", "", "", "", "00000000001" }
};

/* Unsigned Exponential Golomb coding
 */
Bitstream ue(const unsigned int codenum) {
  int x, leading_zeros, nb_bits;
  x = codenum + 1;
  leading_zeros = static_cast<int> (log2(x));
  nb_bits = (leading_zeros << 1) + 1;

  std::string codeword = std::bitset<64>(x).to_string();
  codeword = codeword.substr(64-nb_bits, nb_bits);
  return Bitstream(codeword);
}

/* Signed Exponential Golomb coding
 */
Bitstream se(const int codenum) {
  unsigned int _codenum = (codenum > 0) ? 2*codenum-1 : 2*(-codenum);
  return ue(_codenum);
}

void scan_zigzag(Block4x4 block, int tblock[]) {
  for (int i = 0; i < 16; i++)
    tblock[mat_zigzag4x4[i]] = block[i];
}

void scan_zigzag(Block2x2 block, int tblock[]) {
  tblock[0] = block[0];
  tblock[1] = block[1];
  tblock[2] = block[2];
  tblock[3] = block[3];
}

std::pair<Bitstream, int> cavlc_block4x4(Block4x4 block, const int nC, const int maxNumCoeff) {
  int mat_x[16];
  scan_zigzag(block, mat_x);

  int total_coeff = 0;
  int total_zeros = 0;
  int trail_ones = 0;
  int highest_idx = 0;

  int coeff_table_idx = 5;
  if (nC >= 0 && nC < 2)
    coeff_table_idx = 0;
  else if (nC >= 2 && nC < 4)
    coeff_table_idx = 1;
  else if (nC >= 4 && nC < 8)
    coeff_table_idx = 2;
  else if (nC >= 8)
    coeff_table_idx = 3;
  else if (nC == -1)
    coeff_table_idx = 4;

  // Get the highest frequency coeff
  for (int i = 15; i >= 0; i--) {
    if (mat_x[i] != 0) {
      highest_idx = i;
      break;
    }
  }

  // Count TotalCoeff, TotalZeros
  for (int i = 0; i <= highest_idx; i++) {
    if (mat_x[i] != 0)
      total_coeff++;
  }
  total_zeros = highest_idx - total_coeff + 1;
  if (maxNumCoeff == 15 && total_zeros > 0)
    total_zeros--;

  // Count trailing ones
  std::string ones_str;
  int resume_idx = highest_idx;
  for (int i = highest_idx; i >= 0; i--) {
    if (mat_x[i] != 0) {
      if (mat_x[i] == 1) {
        trail_ones++;
        ones_str += "0";
      }
      else if (mat_x[i] == -1) {
        trail_ones++;
        ones_str += "1";
      }
      else {
        resume_idx = i;
        break;
      }

      if (trail_ones == 3) {
        resume_idx = i - 1;
        break;
      }
    }
  }
  
  // Level encoding
  std::string level_vlc_str = "";
  int lastCoeff = total_coeff - trail_ones;
  if (lastCoeff > 0) {
    int suffix_len = 0;
    bool pad_this = (trail_ones < 3);

    if (total_coeff > 10 && trail_ones < 3)
      suffix_len = 1;

    for (int i = resume_idx; i >= 0; i--) {
      if (mat_x[i] != 0) {
        lastCoeff--;
        int level_code = mat_x[i];

        if (pad_this) {
          if (mat_x[i] > 0)
            level_code -= 1;
          else
            level_code += 1;
          pad_this = false;
        }

        level_code *= 2;
        if (level_code >= 0)
          level_code -= 2;
        else
          level_code = 0 - (level_code + 1);

        int level_prefix = 0;
        std::string level_prefix_str = "";
        bool solution_found = false;

        while (!solution_found) {
          int level_suffix_len = 0;
          if (level_prefix == 14 && suffix_len == 0)
            level_suffix_len = 4;
          else {
            if (level_prefix >= 15)
              level_suffix_len = level_prefix - 3;
            else
              level_suffix_len = suffix_len;
          }

          if (level_prefix >= 16)
            level_code -= (1 << (level_prefix - 3)) - 4096;

          if (level_prefix >= 15 && suffix_len == 0)
            level_code -= 15;

          int level_code_prefix = std::min(15, level_prefix) * pow(2.0, suffix_len);
          int level_suffix = level_code - level_code_prefix;
          int level_max = pow(2.0, level_suffix_len) - 1;

          if (level_suffix <= level_max) {
            solution_found = true;
            level_vlc_str += level_prefix_str + "1";
            if (level_suffix_len != 0) {
              std::string encoded_str;
              if (level_suffix != 0) {
                std::bitset<64> bits(level_suffix);
                encoded_str = bits.to_string();
                int first_one_pos = encoded_str.find_first_of("1");
                encoded_str = encoded_str.substr(first_one_pos, 64 - first_one_pos);
              }
              else
                encoded_str = "0";
              while (encoded_str.length() < (unsigned int)level_suffix_len)
                encoded_str = "0" + encoded_str;
              level_vlc_str += encoded_str;
            }

            if (std::abs(mat_x[i]) > (3 << (suffix_len - 1)) && suffix_len < 6)
              suffix_len++;
            if (lastCoeff == total_coeff - 1 - trail_ones && std::abs(mat_x[i]) > 3)
              suffix_len = 2;
          }
          else {
            level_prefix++;
            level_prefix_str += "0";
          }
        }
      }
    }
  }

  // Calculate run-before
  std::string run_vlc_str = "";
  int last_zeros = total_zeros;
  int coeff_cnt = total_coeff - 1;

  if (total_coeff == maxNumCoeff)
    last_zeros = 0;

  for (int i = 15; i >= 0; i--) {
    if (mat_x[i] != 0) {
      int zero_cnt = 0;
      int j = i - 1;
      for (; j >= 0; j--) {
        if (mat_x[j] != 0)
          break;
        zero_cnt++;
      }

      if (j != -1) {
        std::string run_str = "";
        if (last_zeros <= 6)
          run_str = run_vlc_table[zero_cnt][last_zeros];
        else
          run_str = run_vlc_table[zero_cnt][7];
        last_zeros -= zero_cnt;
        coeff_cnt--;
        run_vlc_str += run_str;
      }
    }

    if (last_zeros == 0)
      break;
  }

  std::string final_str = num_vlc_table[coeff_table_idx][total_coeff][trail_ones] + ones_str + level_vlc_str;
  if (total_coeff < maxNumCoeff)
    final_str += zero_vlc_table[total_zeros][total_coeff];
  final_str += run_vlc_str;


  // if (total_coeff > 0) {
  //  printf("[cavlc4x4] nC = %d, total_coeff = %d, trail_ones = %d\n", nC, total_coeff, trail_ones);
  //  std::cout << "  coeff_token = " << num_vlc_table[coeff_table_idx][total_coeff][trail_ones] << std::endl;
  //  std::cout << "  sign_ones   = " << ones_str << std::endl;
  //  std::cout << "  level       = " << level_vlc_str << std::endl;
  //  std::cout << "  zeros       = " << zero_vlc_table[total_zeros][total_coeff] << std::endl;
  //  std::cout << "  run         = " << run_vlc_str << std::endl;
    // std::cout << Bitstream(final_str).to_string() << std::endl;
  // }

  return std::make_pair(Bitstream(final_str), total_coeff);
}

std::pair<Bitstream, int> cavlc_block2x2(Block2x2 block, const int nC, const int maxNumCoeff) {
  int mat_x[4];
  scan_zigzag(block, mat_x);

  int total_coeff = 0;
  int total_zeros = 0;
  int trail_ones = 0;
  int highest_idx = 0;

  int coeff_table_idx = 5;
  if (nC >= 0 && nC < 2)
    coeff_table_idx = 0;
  else if (nC >= 2 && nC < 4)
    coeff_table_idx = 1;
  else if (nC >= 4 && nC < 8)
    coeff_table_idx = 2;
  else if (nC >= 8)
    coeff_table_idx = 3;
  else if (nC == -1)
    coeff_table_idx = 4;

  // Get the highest frequency coeff
  for (int i = 3; i >= 0; i--) {
    if (mat_x[i] != 0) {
      highest_idx = i;
      break;
    }
  }

  // Count TotalCoeff, TotalZeros
  for (int i = 0; i <= highest_idx; i++) {
    if (mat_x[i] != 0)
      total_coeff++;
  }
  total_zeros = highest_idx - total_coeff + 1;

  // Count trailing ones
  std::string ones_str;
  int resume_idx = highest_idx;
  for (int i = highest_idx; i >= 0; i--) {
    if (mat_x[i] != 0) {
      if (mat_x[i] == 1) {
        trail_ones++;
        ones_str += "0";
      }
      else if (mat_x[i] == -1) {
        trail_ones++;
        ones_str += "1";
      }
      else {
        resume_idx = i;
        break;
      }

      if (trail_ones == 3) {
        resume_idx = i - 1;
        break;
      }
    }
  }
 
  // Level encoding
  std::string level_vlc_str = "";
  int lastCoeff = total_coeff - trail_ones;
  if (lastCoeff > 0) {
    int suffix_len = 0;
    bool pad_this = (trail_ones < 3);

    for (int i = resume_idx; i >= 0; i--) {
      if (mat_x[i] != 0) {
        lastCoeff--;
        int level_code = mat_x[i];

        if (pad_this) {
          if (mat_x[i] > 0)
            level_code -= 1;
          else
            level_code += 1;
          pad_this = false;
        }

        level_code *= 2;
        if (level_code >= 0)
          level_code -= 2;
        else
          level_code = 0 - (level_code + 1);

        int level_prefix = 0;
        std::string level_prefix_str = "";
        bool solution_found = false;

        while (!solution_found) {
          int level_suffix_len = 0;
          if (level_prefix == 14 && suffix_len == 0)
            level_suffix_len = 4;
          else {
            if (level_prefix >= 15)
              level_suffix_len = level_prefix - 3;
            else
              level_suffix_len = suffix_len;
          }

          if (level_prefix >= 16)
            level_code -= (1 << (level_prefix - 3)) - 4096;

          if (level_prefix >= 15 && suffix_len == 0)
            level_code -= 15;

          int level_code_prefix = std::min(15, level_prefix) * pow(2.0, suffix_len);
          int level_suffix = level_code - level_code_prefix;
          int level_max = pow(2.0, level_suffix_len) - 1;

          if (level_suffix <= level_max) {
            solution_found = true;
            level_vlc_str += level_prefix_str + "1";
            if (level_suffix_len != 0) {
              std::string encoded_str;
              if (level_suffix != 0) {
                std::bitset<64> bits(level_suffix);
                encoded_str = bits.to_string();
                int first_one_pos = encoded_str.find_first_of("1");
                encoded_str = encoded_str.substr(first_one_pos, 64 - first_one_pos);
              }
              else
                encoded_str = "0";
              while (encoded_str.length() < (unsigned int)level_suffix_len)
                encoded_str = "0" + encoded_str;
              level_vlc_str += encoded_str;
            }

            if (std::abs(mat_x[i]) > (3 << (suffix_len - 1)) && suffix_len < 6)
              suffix_len++;
            if (lastCoeff == total_coeff - 1 - trail_ones && std::abs(mat_x[i]) > 3)
              suffix_len = 2;
          }
          else {
            level_prefix++;
            level_prefix_str += "0";
          }
        }
      }
    }
  }

  // Calculate run-before
  std::string run_vlc_str = "";
  int last_zeros = total_zeros;
  int coeff_cnt = total_coeff - 1;

  if (total_coeff == maxNumCoeff)
    last_zeros = 0;

  for (int i = 3; i >= 0; i--) {
    if (mat_x[i] != 0) {
      int zero_cnt = 0;
      int j = i - 1;
      for (; j >= 0; j--) {
        if (mat_x[j] != 0)
          break;
        zero_cnt++;
      }

      if (j != -1) {
        std::string run_str = "";
        if (last_zeros <= 6)
          run_str = run_vlc_table[zero_cnt][last_zeros];
        else
          run_str = run_vlc_table[zero_cnt][7];
        last_zeros -= zero_cnt;
        coeff_cnt--;
        run_vlc_str += run_str;
      }
    }

    if (last_zeros == 0)
      break;
  }

  std::string final_str = num_vlc_table[coeff_table_idx][total_coeff][trail_ones] + ones_str + level_vlc_str;
  if (total_coeff < maxNumCoeff)
    final_str += zero_vlc_table2x2[total_zeros][total_coeff];
  final_str += run_vlc_str;

  // if (total_coeff > 0) {
  //  printf("[cavlc2x2] nC = %d, total_coeff = %d, trail_ones = %d\n", nC, total_coeff, trail_ones);
  //  std::cout << "  coeff_token = " << num_vlc_table[coeff_table_idx][total_coeff][trail_ones] << std::endl;
  //  std::cout << "  sign_ones   = " << ones_str << std::endl;
  //  std::cout << "  level       = " << level_vlc_str << std::endl;
  //  std::cout << "  zeros       = " << zero_vlc_table2x2[total_zeros][total_coeff] << std::endl;
  //  std::cout << "  run         = " << run_vlc_str << std::endl;
  // }

  return std::make_pair(Bitstream(final_str), total_coeff);
}

