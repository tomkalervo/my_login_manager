#include "login_manager.h"
#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __APPLE__
#include <sys/_types/_socklen_t.h>
#else
#include <atomic>
#endif

#define PORT 1717
using std::string;
std::atomic_bool stop_api_thread = false;
void printBits(unsigned int num) {
  int numBits = sizeof(num) * 8; // Number of bits in the integer type
  for (int i = numBits - 1; i >= 0; --i) {
    std::cout << ((num >> i) & 1);
  }
}
int sendMsg(int sockfd, const struct sockaddr_in &servaddr, const char *msg,
            int msg_len) {
  if (sendto(sockfd, msg, msg_len, 0,
             reinterpret_cast<const struct sockaddr *>(&servaddr),
             sizeof(servaddr)) < 0) {
    std::cerr << "Error sending message\n";
    close(sockfd);
    exit(0);
  };
  // Receive response
  char buffer[1024];
  struct sockaddr_in from_addr;
  socklen_t from_len = sizeof(from_addr);
  ssize_t n =
      recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
               reinterpret_cast<struct sockaddr *>(&from_addr), &from_len);
  if (n < 0) {
    std::cerr << "Error receiving response\n";
    close(sockfd);
    exit(0);
  }
  if (n >= 1024) {
    std::cerr << "Buffer maxsize overflow" << std::endl;
    close(sockfd);
    exit(0);
  }
  // Check if the sender address matches the server address
  if (from_addr.sin_family == servaddr.sin_family &&
      from_addr.sin_addr.s_addr == servaddr.sin_addr.s_addr &&
      from_addr.sin_port == servaddr.sin_port) {
    std::cout << "Received response from the expected server." << std::endl;
  } else {
    std::cerr << "Received response from an unexpected server." << std::endl;
  }

  // Null-terminate the buffer and print it
  buffer[n] = '\0';

  // Print hex values of the first 8 bytes in the buffer
  int rc = 0;
  std::cout << "First " << n << " bytes in hex: ";
  for (int i = 0; i < n; ++i) {
    rc |= static_cast<unsigned char>(buffer[i]) << (i * sizeof(unsigned char));
    std::cout << std::hex << std::uppercase
              << static_cast<int>(static_cast<unsigned char>(buffer[n - i - 1]))
              << " ";
  }
  std::cout << std::endl;

  return rc;
}
void startApi() {
  LoginManager lm("../database/login.db");
  std::cout << "API server opened.\n";
  lm.startAPI();
  std::cout << "API server closed.\n";
}
int testDel(string uname, string passw, int sockfd,
            struct sockaddr_in servaddr) {

  uint8_t op = 4;
  uint16_t uname_len = uname.length();
  uint16_t passw_len = passw.length();
  int msg_len = sizeof(uint8_t) + sizeof(uint16_t) + uname_len +
                sizeof(uint16_t) + passw_len;
  char msg[msg_len];
  int idx = 0;
  msg[idx] = static_cast<char>(op);
  idx += sizeof(uint8_t);

  std::memcpy(msg + idx, &uname_len, sizeof(uname_len));
  idx += sizeof(uint16_t);

  std::memcpy(msg + idx, uname.data(), uname_len);
  idx += uname_len;

  std::memcpy(msg + idx, &passw_len, sizeof(passw_len));
  idx += sizeof(uint16_t);

  std::memcpy(msg + idx, passw.data(), passw_len);

  int rc = sendMsg(sockfd, servaddr, msg, msg_len);
  return rc;
}

int testAdd(string uname, string passw, int sockfd,
            struct sockaddr_in servaddr) {

  uint8_t op = 3;
  uint16_t uname_len = uname.length();
  uint16_t passw_len = passw.length();
  int msg_len = sizeof(uint8_t) + sizeof(uint16_t) + uname_len +
                sizeof(uint16_t) + passw_len;
  char msg[msg_len];
  int idx = 0;
  msg[idx] = static_cast<char>(op);
  idx += sizeof(uint8_t);

  std::memcpy(msg + idx, &uname_len, sizeof(uname_len));
  idx += sizeof(uint16_t);

  std::memcpy(msg + idx, uname.data(), uname_len);
  idx += uname_len;

  std::memcpy(msg + idx, &passw_len, sizeof(passw_len));
  idx += sizeof(uint16_t);

  std::memcpy(msg + idx, passw.data(), passw_len);

  int rc = sendMsg(sockfd, servaddr, msg, msg_len);
  return rc;
}
int testLogin(string uname, string passw, int sockfd,
              struct sockaddr_in servaddr) {

  std::cout << "01 uname string: " << uname << "\n";
  std::cout << "01 passw string: " << passw << "\n";
  uint8_t op = 1;
  uint16_t uname_len = uname.length();
  uint16_t passw_len = passw.length();
  int msg_len = sizeof(uint8_t) + sizeof(uint16_t) + uname_len +
                sizeof(uint16_t) + passw_len;
  char msg[msg_len];
  int idx = 0;
  msg[idx] = static_cast<char>(op);
  idx += sizeof(uint8_t);

  std::memcpy(msg + idx, &uname_len, sizeof(uname_len));
  idx += sizeof(uint16_t);

  std::memcpy(msg + idx, uname.data(), uname_len);
  idx += uname_len;

  std::memcpy(msg + idx, &passw_len, sizeof(passw_len));
  idx += sizeof(uint16_t);

  std::memcpy(msg + idx, passw.data(), passw_len);

  int rc = sendMsg(sockfd, servaddr, msg, msg_len);
  return rc;
}
void testApi() {
  // Set up UDP socket
  int sockfd;
  struct sockaddr_in servaddr;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Could not bind socket: ");
    return;
  }
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Send & Recieve
  string uname = "";
  string passw = "";
  uname = "testtom@mail.io";
  passw = "testpassw1234";
  int rc = testLogin(uname, passw, sockfd, servaddr);
  if (rc == 0x00000000) {
    std::cout << "01 API Login correct usernamne & password test passed."
              << std::endl;
  } else {
    std::cout << "01 API Login correct usernamne & password test failed. RC: ";
    printBits(rc);
    std::cout << std::endl;
  }

  uname = "testtom@mail.io";
  passw = "testpassw1230";
  rc = testLogin(uname, passw, sockfd, servaddr);
  if (rc == 0x00000001) {
    std::cout << "02 API Login bad usernamne & password test passed."
              << std::endl;
  } else {
    std::cout << "02 API Login bad usernamne & password test failed. RC: ";
    printBits(rc);
    std::cout << std::endl;
  }

  uname = "testadd@mail.io";
  passw = ")*Hk&1230!-test";
  rc = testAdd(uname, passw, sockfd, servaddr);
  if (rc == 0x00000000) {
    std::cout << "03 API Add usernamne & password test passed." << std::endl;
  } else {
    std::cout << "03 API Add usernamne & password test failed. RC: ";
    printBits(rc);
    std::cout << std::endl;
  }

  rc = testLogin(uname, passw, sockfd, servaddr);
  if (rc == 0x00000000) {
    std::cout << "04 API Login with added usernamne & password test passed."
              << std::endl;
  } else {
    std::cout
        << "04 API Login with added usernamne & password test failed. RC: ";
    printBits(rc);
    std::cout << std::endl;
  }

  rc = testDel(uname, passw, sockfd, servaddr);
  if (rc == 0x00000000) {
    std::cout << "05 API Delete of added usernamne & password test passed."
              << std::endl;
  } else {
    std::cout
        << "05 API Delete of added usernamne & password test failed. RC: ";
    printBits(rc);
    std::cout << std::endl;
  }
  close(sockfd);
}

int main() {
  LoginManager lm("../database/login.db");
  lm.startAPI();
  std::cout << "API server opened.\n";
  testApi();
  sleep(1);
  std::cout << "Closing API server.\n";
  lm.stopAPI();
  std::cout << "API server closed.\n";
  return 0;
}
