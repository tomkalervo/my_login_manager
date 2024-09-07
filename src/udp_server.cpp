/**
 * Author: Tom Karlsson
 * 2024-08-11 23:37
 *
 * This static class adds an API to the Login Manager.
 * When activated, it can be reached through a local socket (127.0.0.1) at port
 * 1717. It uses Datagrams, UDP as the transport protocol.
 *
 */

#include "udp_server.h"
#include "login_manager.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#define PORT 1717
#define MAXLINE 1024
/*
 * Incoming bytearray starts with operation code {1 byte, usigned integer}
 * For every parameter: Length {4 bytes, unsigned integer}, Value {datatype
 * specified by operation}
 *
 * Operations.
 * 1 : Login with e-mail(e-mail {string utf8}, password {string utf8})
 * 2 : Login with username, TODO
 * 3 : Add user(e-mail {string utf8}, password {string utf8})
 * 4 : Delete user with e-mail(e-mail {string utf8}, password {string utf8})
 * 5 : Delete user with username, TODO
 * Return codes.
 * bit 1: represents api communication {0 = OK | 1 = not OK}
 * bit 2-7 represents reason.
 * [0]+[000 0000] : Server ok, operation success
 * [0]+[000 0001] : Server ok, operation failure
 * [1]+[111 1111] : Server error, check log
 * [1]+[010 0000] : API error, invalid operation code
 * [1]+[001 0000] : API error, invalid parameter
 */
enum RC_API_OK { TRANS_SUCCESS = 0b0, TRANS_FAILURE = 0b1, TRANS_ERROR = 0b10 };
enum RC_API_ER {
  DATAGRAM_ER = 0b1100 << 4,
  OP_CODE_ER = 0b1010 << 4,
  PARAMETER_ER = 0b1001 << 4
};
int udpServer::getIntVal(const char *msg, const int size) {
  // Presumes little-endian order.
  int val = 0;
  for (int i = 0; i < size; i++) {
    val |= (unsigned char)msg[i] << (i * sizeof(unsigned char));
  }
  return val;
}
int udpServer::opDel(Operation &op) {
  // Username at first parameter
  if (op.idx + 2 >= MAXLINE) {
    return PARAMETER_ER;
  }
  int len = getIntVal(&op.msg[op.idx], 2);
  op.idx += 2;
  if (op.idx + len >= MAXLINE) {
    return PARAMETER_ER;
  }
  string uname = string(&op.msg[op.idx], len);
  op.idx += uname.length();
  // Password at second  parameter
  if (op.idx + 2 >= MAXLINE) {
    return PARAMETER_ER;
  }
  len = getIntVal(&op.msg[op.idx], 2);
  op.idx += 2;
  if (op.idx + len >= MAXLINE) {
    return PARAMETER_ER;
  }
  string passw = string(&op.msg[op.idx], len);
  op.idx += passw.length();
  // Run login procedure
  int rc = op.lm.delLogin(uname, passw);
  if (rc == 0) {
    return TRANS_SUCCESS;
  } else if (rc > 0) {
    return TRANS_FAILURE;
  } else {
    return TRANS_ERROR;
  }
}
int udpServer::opAdd(Operation &op) {
  // Username at first parameter
  if (op.idx + 2 >= MAXLINE) {
    return PARAMETER_ER;
  }
  int len = getIntVal(&op.msg[op.idx], 2);
  op.idx += 2;
  if (op.idx + len >= MAXLINE) {
    return PARAMETER_ER;
  }
  string uname = string(&op.msg[op.idx], len);
  op.idx += uname.length();
  // Password at second  parameter
  if (op.idx + 2 >= MAXLINE) {
    return PARAMETER_ER;
  }
  len = getIntVal(&op.msg[op.idx], 2);
  op.idx += 2;
  if (op.idx + len >= MAXLINE) {
    return PARAMETER_ER;
  }
  string passw = string(&op.msg[op.idx], len);
  op.idx += passw.length();
  // Run login procedure
  int rc = op.lm.addLogin(uname, passw);
  if (rc == 0) {
    return TRANS_SUCCESS;
  } else if (rc > 0) {
    return TRANS_FAILURE;
  } else {
    return TRANS_ERROR;
  }
}
int udpServer::opLogin(Operation &op) {
  // Username at first parameter
  if (op.idx + 2 >= MAXLINE) {
    return PARAMETER_ER;
  }
  int len = getIntVal(&op.msg[op.idx], 2);
  op.idx += 2;
  if (op.idx + len >= MAXLINE) {
    return PARAMETER_ER;
  }
  string uname = string(&op.msg[op.idx], len);
  op.idx += uname.length();
  // Password at second  parameter
  if (op.idx + 2 >= MAXLINE) {
    return PARAMETER_ER;
  }
  len = getIntVal(&op.msg[op.idx], 2);
  op.idx += 2;
  if (op.idx + len >= MAXLINE) {
    return PARAMETER_ER;
  }
  string passw = string(&op.msg[op.idx], len);
  op.idx += passw.length();
  // Run login procedure
  int rc = op.lm.login(uname, passw);
  if (rc == 0) {
    return TRANS_SUCCESS;
  } else if (rc > 0) {
    return TRANS_FAILURE;
  } else {
    return TRANS_ERROR;
  }
}

int udpServer::process_msg(Operation &op) {
  switch ((unsigned short)op.msg[op.idx++]) {
  case 0:
    // no-op
    return 0x7FFFFFFF;
  case 1:
    return opLogin(op);
  case 3:
    return opAdd(op);
  case 4:
    return opDel(op);
  default:
    return OP_CODE_ER;
  }
}

// this function is operated async and must handle memory allocated
void udpServer::handle_client(Operation *op, const int sockfd) {
  int rc = process_msg(*op);
  // std::cerr << "Debug - RC Value: " << rc << std::endl;
  // std::cerr << "Debug - RC Hex Value: 0x" << std::hex << rc << std::dec
  //          << std::endl;
  sendto(sockfd, (const char *)&rc, sizeof(int), 0, op->cliaddr, op->addr_len);
  delete op;
}

// Listens to incoming datagrams. Starts a new thread to handle requests.
void udpServer::listen(int sockfd, sockaddr_in servaddr, Status *st,
                       LoginManager &lm) {
  socklen_t len;
  int n;
  // Stop-bit @ [_ _ _ ?  _ _ _ _]
  while (st->control & 0x10) {
    char *buffer = new char[MAXLINE];
    struct sockaddr_in *client_address = new struct sockaddr_in;
    len = sizeof(*client_address);
    memset(client_address, 0, len);
    n = recvfrom(sockfd, buffer, MAXLINE, MSG_TRUNC,
                 (struct sockaddr *)client_address, &len);
    if (n < 0) {
      std::cerr << "recvfrom failed: " << strerror(errno) << std::endl;
      delete[] buffer;
      delete client_address;
      continue;
    }

    if (n > MAXLINE) {
      std::cerr << "Received datagram exceeds maximum allowed size. Ignoring."
                << std::endl;
      int rc = DATAGRAM_ER;
      sendto(sockfd, (const char *)&rc, sizeof(int), 0,
             (struct sockaddr *)&client_address, len);
      delete[] buffer;
      delete client_address;
      continue;
    }

    buffer[n] = '\0';
    Operation *op = new Operation(buffer, lm, client_address);
    std::thread t(handle_client, op, sockfd);
    t.detach();

    // Check stop-request-bit @ [_ _ _ _  _ _ _ ?]
    st->mtx.lock();
    if (st->control & 0x1) {
      st->control &= ~0x10;
    }
    st->mtx.unlock();
  }

  close(sockfd);
}

// Spin up the API server. Returns a void pointer to the Status struct that is
// used for multithreading communication.
void *udpServer::run(LoginManager &lm) {
  int sockfd;
  struct sockaddr_in servaddr;

  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    return nullptr;
  }

  memset(&servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;
  // Only listen to local requests
  servaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
  servaddr.sin_port = htons(PORT);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    return nullptr;
  }
  Status *st = new Status();
  std::thread t(listen, sockfd, servaddr, st, std::ref(lm));
  t.detach();
  return static_cast<void *>(st);
}

// Takes pointer to the Status struct that handle the multithreading
// communication. Stops the server by setting the stop-request-bit. Returns
// false (0) if unsuccessfull or true (1) if successfull
int udpServer::stop(void *st) {
  if (st == nullptr) {
    return 0;
  }
  // Set the stop-bit of the server
  Status *status = static_cast<Status *>(st);
  status->mtx.lock();
  status->control |= 0x1;
  status->mtx.unlock();

  // Create socket to sent no-op to server.
  int sockfd;
  struct sockaddr_in servaddr;
  int noop = 0;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
    return 0;
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  int it = 0;
  while ((status->control & 0x10) == 0x10) {
    if (it > 10) {
      std::cerr << "Could not stop API server within current timeframe."
                << std::endl;
      return 0;
    }
    // Send noop
    if (sendto(sockfd, &noop, sizeof(noop), 0, (struct sockaddr *)&servaddr,
               sizeof(servaddr)) < 0) {
      perror("Send failed");
      close(sockfd);
      return 0;
    }
    sleep(1);
    it++;
  }

  close(sockfd);
  delete (status);
  return 1;
}
