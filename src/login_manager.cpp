#include "login_manager.h"
#include "database.h"
#include "hash_password.h"
#include "udp_server.h"
#include <iostream>
#include <stdexcept>

const std::string LoginManager::s_salt = "42";
LoginManager::LoginManager(const std::string &dbFile) try : db(dbFile.c_str()) {
  // no-op, init successul
} catch (const std::runtime_error &e) {
  std::cerr << "Failed to initialize database in LoginManager: " << e.what() << std::endl;
  throw; 	
}

void LoginManager::startAPI() { api_status = udpServer::run(*this); }
void LoginManager::stopAPI() {
  int rc = udpServer::stop(api_status);
  if (rc) {
    std::cerr << "Server closed successfully." << std::endl;
  } else {

    std::cerr << "Server failed to close." << std::endl;
  }
}

int LoginManager::login(const std::string &username,
                        const std::string &password) {
  std::string hash_pw;
  if (!getHashedPassword(username, password, hash_pw)) {
    std::cerr << "Error getting hashed password" << std::endl;
    return -1;
  }
  return db.checkPassword(username, hash_pw);
}

int LoginManager::addLogin(const std::string &username,
                           const std::string &password) {
  std::string d_salt;
  genSalt(d_salt);
  if (d_salt.empty()) {
    return -1;
  }

  std::string hashedPassword =
      HashPassword::usingSHA256(s_salt + password + d_salt);
  if (hashedPassword.empty()) {
    return -2;
  }
  return db.addUser(username, hashedPassword, d_salt);
}
int LoginManager::delLogin(const std::string &username,
                           const std::string &password) {
  std::string hash_pw;
  if (!getHashedPassword(username, password, hash_pw)) {
    return -1;
  }
  return db.deleteUser(username, hash_pw);
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
