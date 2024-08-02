/**
 * Code inpsired by geekforgeeks.org
 * https://www.geeksforgeeks.org/udp-server-client-implementation-c/
 */

// Server side implementation of UDP client-server model
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
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
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
 *
 * Return codes.
 * bit 1: {0 = OK | 1 = not OK}
 * bit 2-7 represents reason.
 */
int udpServer::get_int_val(const char *msg, const int size) {
  int val = 0;
  for (int i = 0; i < size; i++) {
    val |= (unsigned short)msg[i] << (sizeof(short) * (size - i - 1));
  }
  return val;
}
int udpServer::process_msg(const char *msg, LoginManager &lm) {
  int i = 0;
  unsigned short op = (unsigned short)msg[i++];
  if (op == 0) {
    std::cerr << "Bad op: " << op << std::endl;
    return (0x1 << 7) | 0x1;
  }
  if (op == 1) {
    if (i + 2 >= MAXLINE) {
      return EXIT_FAILURE;
    }
    int len = get_int_val(&msg[i], 2);
    i += 2;
    if (i + len >= MAXLINE) {
      return EXIT_FAILURE;
    }
    string uname = string(&msg[i], len);
    i += uname.length();

    if (i + 2 >= MAXLINE) {
      return EXIT_FAILURE;
    }
    len = get_int_val(&msg[i], 2);
    i += 2;
    if (i + len >= MAXLINE) {
      return EXIT_FAILURE;
    }
    string passw = string(&msg[i], len);
    i += passw.length();
    std::cerr << "pass: " << passw << std::endl;
    return lm.login(uname, passw);
  }
  return 0x1 << 7;
}
int udpServer::listen(LoginManager &lm) {
  int sockfd;
  char buffer[MAXLINE];
  struct sockaddr_in servaddr, cliaddr;

  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;
  // Only listen to local requests
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = htons(PORT);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    return (EXIT_FAILURE);
  }

  socklen_t len;
  int n;
  int rc = 0;

  len = sizeof(cliaddr); // len is value/result
  while (rc != 1) {
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                 (struct sockaddr *)&cliaddr, &len);
    buffer[n] = '\0';
    int rc = process_msg(buffer, lm);
    printf("Client : %s\n", buffer);
    printf("RC : %d\n", rc);
    sendto(sockfd, (const char *)&rc, sizeof(int), 0,
           (const struct sockaddr *)&cliaddr, len);
  }

  close(sockfd);
  return EXIT_SUCCESS;
}
