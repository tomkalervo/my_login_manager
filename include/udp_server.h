#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include "login_manager.h"
class udpServer {
public:
  static int listen(LoginManager &lm);

private:
  static int process_msg(const char *msg, LoginManager &lm);
  static int get_int_val(const char *msg, const int size);
};

#endif // ! UDP_SERVER_H
