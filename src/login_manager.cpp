#include "login_manager.h"
#include <iostream>

const std::string LoginManager::s_salt = "42";
LoginManager::LoginManager(const std::string &dbFile) : db(dbFile.c_str()) {}

int LoginManager::login(const std::string &username,
                        const std::string &password) {
  std::string hash_pw;
  if (!getHashedPassword(username, password, hash_pw)) {
    std::cerr << "Error getting hashed password" << std::endl;
    return -1;
  }
  int rc = db.checkPassword(username, hash_pw);
  if (rc <= 0) {
    return 0 - rc;
  } else {
    std::cerr << "Error checking password in database" << std::endl;
    return -1;
  }
}

bool LoginManager::addLogin(const std::string &username,
                            const std::string &password) {
  std::string d_salt;
  genSalt(d_salt);
  if (d_salt.empty()) {
    return false;
  }

  std::string hashedPassword =
      HashPassword::usingSHA256(s_salt + password + d_salt);
  if (hashedPassword.empty()) {
    return false;
  }

  if (db.addUser(username, hashedPassword, d_salt) == 0) {
    return true;
  }
  return false;
}

bool LoginManager::delLogin(const std::string &username,
                            const std::string &password) {
  std::string hash_pw;
  if (!getHashedPassword(username, password, hash_pw)) {
    return false;
  }

  if (db.deleteUser(username, hash_pw) == 0) {
    return true;
  }
  return false;
}

bool LoginManager::getHashedPassword(const std::string &usid,
                                     const std::string &pw,
                                     std::string &hashed_pw) {
  std::string d_salt;
  if (!getSalt(usid, d_salt) || d_salt.empty()) {
    return false;
  }
  hashed_pw = HashPassword::usingSHA256(s_salt + pw + d_salt);
  return !hashed_pw.empty();
}
bool LoginManager::getSalt(const std::string &username, std::string &salt) {
  return (db.getUserSalt(username, salt) == 0);
}
void LoginManager::genSalt(std::string &salt) { salt = "56"; }
void LoginManager::hash(const std::string &input, std::string &output) {
  // Dummy hash function that just assigns input to output for demonstration
  output = "hashed_" + input;
}
