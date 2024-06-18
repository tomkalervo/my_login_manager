#include <hash_password.h>
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <cstdio>   // For snprint

const uint32_t HashPassword::h_init[8] = {
  0x6a09e667,
  0xbb67ae85,
  0x3c6ef372,
  0xa54ff53a,
  0x510e527f,
  0x9b05688c,
  0x1f83d9ab,
  0x5be0cd19
};
const uint32_t HashPassword::k[64] = {
   0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

std::vector<uint8_t> HashPassword::padd(const std::string& text){
  uint64_t len = text.length() * 8;
  size_t padd_len = (512 - ((len + 65) % 512)) % 512;
  size_t tot_bytes = ((len + 65) + padd_len) / 8;
  std::vector<uint8_t> bytes(tot_bytes, 0x00);
  std::copy(text.begin(), text.end(), bytes.begin());
  // Append single '1' bit. 0x80 = 0b10000000
  bytes[text.length()] = 0x80;
  // 0-bit padding already applied during init
  // Add big-endian integer of text length
  for (int i = 0; i < 8; i++) {
    bytes[tot_bytes - 1 - i] = static_cast<uint8_t>(len >> (8 * i));
  }
  return bytes;
}
uint32_t HashPassword::rrot(const uint32_t v, const uint32_t n){
  return (v >> n) | (v << (32 - n));
}
std::string HashPassword::usingSHA256(const std::string& text){
  std::vector<uint8_t> padded_text = HashPassword::padd(text);

  uint32_t hash[8];
  for(int i = 0; i < 8; i++){hash[i] = h_init[i];}

  uint32_t w[64] = {};
  // work on 512 bit chunks
  for (size_t chunk = 0; chunk < padded_text.size(); chunk += 64) {
    for (size_t i = 0; i < 16; i++) {
      w[i] = 
          (padded_text[chunk + (i*4) + 0] << 3*8)
        | (padded_text[chunk + (i*4) + 1] << 2*8)
        | (padded_text[chunk + (i*4) + 2] << 1*8)
        | (padded_text[chunk + (i*4) + 3] << 0*8);
    }
    for (size_t i = 16; i < 64; i++) {
      uint32_t s0 = (rrot(w[i-15], 7) ^ rrot(w[i-15], 18)) ^ (w[i-15] >> 3);
      uint32_t s1 = (rrot(w[i-2], 17) ^ rrot(w[i-2], 19))  ^ (w[i-2] >> 10);
      w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    uint32_t a = hash[0];
    uint32_t b = hash[1];
    uint32_t c = hash[2];
    uint32_t d = hash[3];
    uint32_t e = hash[4];
    uint32_t f = hash[5];
    uint32_t g = hash[6];
    uint32_t h = hash[7];
    // Compression
    for (size_t i = 0; i < 64; i++) {
      uint32_t sum1 = (rrot(e, 6) ^ rrot(e, 11)) ^ rrot(e, 25);
      uint32_t chce = (e & f) ^ (~e & g);
      uint32_t tmp1 = h + sum1 + chce + HashPassword::k[i] + w[i];
      uint32_t sum2 = (rrot(a, 2) ^ rrot(a, 13)) ^ rrot(a, 22);
      uint32_t majy = (a & b) ^ (a & c) ^ (b & c);
      uint32_t tmp2 = sum2 + majy;
    
      h = g;
      g = f;
      f = e;
      e = d + tmp1;
      d = c;
      c = b;
      b = a;
      a = tmp1 + tmp2;
    }

    //add to hash
    hash[0] += a;
    hash[1] += b;
    hash[2] += c;
    hash[3] += d;
    hash[4] += e;
    hash[5] += f;
    hash[6] += g;
    hash[7] += h;

  }
  std::string result;
  result.reserve(128);
  for(const auto& bytes : hash) {
    char hex_byte[9];
    snprintf(hex_byte, 9, "%08x", bytes);
    result += hex_byte;
  }
  return result;
}
