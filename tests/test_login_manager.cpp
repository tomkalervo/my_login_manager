#include "login_manager.h"
#include <iostream>
#include <string>

void testLogin() {
  LoginManager lm("../database/login.db");

  const std::string login_secid = "test123@mail.io";
  const std::string login_pw = "1234PassWord";
  if (lm.addLogin(login_secid, login_pw) == 0) {
    std::cout << "01 Add login test passed." << std::endl;
  } else {
    std::cout << "01 Add login test failed." << std::endl;
  }

  int rc = lm.login(login_secid, login_pw);
  if (rc == 0) {
    std::cout << "02 Login test passed." << std::endl;
  } else if (rc == 1) {
    std::cout << "02 Login test failed." << std::endl;
  } else {
    std::cout << "02 Login test rc error." << std::endl;
  }

  const std::string login_pw_false = "1234passWord";
  rc = lm.login(login_secid, login_pw_false);
  if (rc == 12) { // 12 is Not found by Sqlite3
    std::cout << "03 False login test passed." << std::endl;
  } else {
    std::cout << "03 False login test failed. rc: " << rc << std::endl;
  }

  if (lm.delLogin(login_secid, login_pw) == 0) {
    std::cout << "04 Delete login test passed." << std::endl;
  } else {
    std::cout << "04 Delete login test failed." << std::endl;
  }

  const std::string login_secid2 = "test002@mail.io";
  const std::string login_pw2 = "0002PassWord";
  if (lm.addLogin(login_secid2, login_pw2) == 0) {
    std::cout << "05 Add login2 test passed." << std::endl;
  } else {
    std::cout << "05 Add login2 test failed." << std::endl;
  }

  const std::string login_secid3 = "test003@mail.io";
  const std::string login_pw3 = "0003PassWord";
  if (lm.addLogin(login_secid3, login_pw3) == 0) {
    std::cout << "06 Add login3 test passed." << std::endl;
  } else {
    std::cout << "06 Add login3 test failed." << std::endl;
  }

  rc = lm.login(login_secid2, login_pw2);
  if (rc == 0) {
    std::cout << "07 Login2 test passed." << std::endl;
  } else if (rc == 1) {
    std::cout << "07 Login2 test failed." << std::endl;
  } else {
    std::cout << "07 Login2 test rc error." << std::endl;
  }

  rc = lm.login(login_secid3, login_pw3);
  if (rc == 0) {
    std::cout << "08 Login3 test passed." << std::endl;
  } else if (rc == 1) {
    std::cout << "08 Login3 test failed." << std::endl;
  } else {
    std::cout << "08 Login3 test rc error." << std::endl;
  }

  std::string const login_pw2_changed = "002ChangedPassw";
  rc = lm.login(login_secid2, login_pw2_changed);
  if (rc != 0) {
    std::cout << "09 login2 with unchanged new password test passed."
              << std::endl;
  } else {
    std::cout << "09 login2 with unchanged new password test failed."
              << std::endl;
  }

  rc = lm.changePassword(login_secid2, login_pw2_changed);
  if (rc == 0) {
    std::cout << "10 login2 change new password test passed." << std::endl;
  } else {
    std::cout << "10 login2 changed new password test failed." << std::endl;
  }

  rc = lm.login(login_secid2, login_pw2_changed);
  if (rc == 0) {
    std::cout << "11 login2 with changed new password test passed."
              << std::endl;
  } else {
    std::cout << "11 login2 with changed new password test failed."
              << std::endl;
  }

  if (lm.delLogin(login_secid2, login_pw2_changed) == 0) {
    std::cout << "12 Delete login2 test passed." << std::endl;
  } else {
    std::cout << "12 Delete login2 test failed." << std::endl;
  }

  if (lm.delLogin(login_secid3, login_pw3) == 0) {
    std::cout << "13 Delete login3 test passed." << std::endl;
  } else {
    std::cout << "13 Delete login3 test failed." << std::endl;
  }
}

int main() {
  testLogin();
  return 0;
}
