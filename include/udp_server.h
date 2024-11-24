#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include "login_manager.h"
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
class udpServer {
public:
  static void *run(LoginManager &lm);
  static int stop(void *st);

private:
  struct Status {
    int control;
    int current_transactions;
    std::mutex mtx;
    Status() : control(0x10), current_transactions(0) {}
  };
  struct Operation {
    int idx;
    const char *msg;
    LoginManager &lm;
    const struct sockaddr *cliaddr;
    int addr_len;
    Operation(const char *message, LoginManager &login_manager,
              struct sockaddr_in *client_address)
        : idx(0), msg(message), lm(login_manager) {
      cliaddr = (const struct sockaddr *)client_address;
      addr_len = sizeof(struct sockaddr_in);
    }
    ~Operation() {
      delete[] msg;
      delete cliaddr;
    }
  };
  static void listen(int sockfd, sockaddr_in servaddr, Status *st,
                     LoginManager &lm);
  static void handle_client(Operation *op, const int sockfd);
  static int addTransaction(Status &st);
  static int delTransaction(Status &st);
  static int getTransactions(Status &st);
  static int getControl(Status &st);
  static int setControl(Status &st);
  static int process_msg(Operation &op);
  static int getIntVal(const char *msg, const int size);
  static string getStringVal(const char *msg, const int size);
  static int opLogin(Operation &op);
  static int opAdd(Operation &op);
  static int opDel(Operation &op);
  static int opModPassw(Operation &op);
};

#endif // ! UDP_SERVER_H
