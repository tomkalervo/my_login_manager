/* HashPassword takes a user password as a string and returns a hashed string using the SHA256 algorithm.
 * Coded by Tom Karlsson
 * 
 * The code is entirely inspired by the wikipedia description:
 * https://en.wikipedia.org/wiki/SHA-2 
 *
 */
#ifndef HASH_PASSWORD
#define HASH_PASSWORD
#include <string>

class HashPassword{
public:
  static std::string usingSHA256(const std::string& text);
private:
  const static uint32_t h_init[8];
  const static uint32_t k[64];
  static std::vector<uint8_t> padd(const std::string& text);
  static uint32_t rrot(const uint32_t v, const uint32_t n);

};
#endif 
