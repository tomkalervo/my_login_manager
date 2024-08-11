#include "login_manager.h"
#include <cstdint>
#include <iostream>
#include <string>
using std::string;
std::atomic_bool stop_api_thread = false;
void printBits(unsigned int num) {
  int numBits = sizeof(num) * 8; // Number of bits in the integer type
  for (int i = numBits - 1; i >= 0; --i) {
    std::cout << ((num >> i) & 1);
  }
}
int sendMsg(const char *msg) { return 0x1111000; }
void startApi() {
  LoginManager lm("../database/login.db");
  std::cout << "API server opened.\n";
  lm.startAPI();
  std::cout << "API server closed.\n";
}
void testApi() {
  const string uname = "";
  const string passw = "";
  std::cout << "01 uname string: " << uname << "\n";
  std::cout << "01 passw string: " << passw << "\n";
  uint8_t op = 1;
  uint16_t uname_len = uname.length();
  uint16_t passw_len = passw.length();
  char *msg = new char[sizeof(uint8_t) + sizeof(uint16_t) + uname_len +
                       sizeof(uint16_t) + passw_len];
  int idx = 0;
  msg[idx] = static_cast<char>(op);
  idx += sizeof(uint8_t);

  std::memcpy(msg + idx, &uname_len, sizeof(uint16_t));
  idx += sizeof(uint16_t);

  std::memcpy(msg + idx, uname.data(), uname_len);
  idx += uname_len;

  std::memcpy(msg + idx, &passw_len, sizeof(uint16_t));
  idx += sizeof(uint16_t);

  std::memcpy(msg + idx, passw.data(), passw_len);

  int rc = sendMsg(msg);

  if (rc == 0x00000000) {
    std::cout << "01 API Login correct usernamne & password test passed."
              << std::endl;
  } else {
    std::cout << "01 API Login correct usernamne & password test failed. RC: ";
    printBits(rc);
    std::cout << std::endl;
  }
}

int main() {
  LoginManager lm("../database/login.db");
  std::cout << "API server opened.\n";
  lm.startAPI();
  std::cout << "API server closed.\n";
  testApi();
  lm.stopAPI();
  return 0;
}
