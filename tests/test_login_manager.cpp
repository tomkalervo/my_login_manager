#include <iostream>
#include <string>
#include "login_manager.h"

void testLogin() {
  LoginManager lm("database/login.db");
  
  const std::string login_secid = "test123@mail.io";
  const std::string login_pw = "1234PassWord"; 
  if (lm.addLogin(login_secid, login_pw)) {
      std::cout << "01 Add login test passed." << std::endl;
  } else {
      std::cout << "01 Add login test failed." << std::endl;
  }
  
  if (lm.login(login_secid,login_pw)) {
      std::cout << "02 Login test passed." << std::endl;
  } else {
      std::cout << "02 Login test failed." << std::endl;
  }
  
  if (lm.delLogin(login_secid,login_pw)) {
      std::cout << "03 Delete login test passed." << std::endl;
  } else {
      std::cout << "03 Delete login test failed." << std::endl;
  }
}

int main() {
    testLogin();
    return 0;
}

