#include "login_manager.h"
#include <iostream>

const std::string LoginManager::s_salt = "42";
LoginManager::LoginManager(const std::string& dbFile) : db(dbFile.c_str()) {}

bool LoginManager::login(const std::string& username, const std::string& password) {
  std::string hash_pw;
  if (!getHashedPassword(username,password,hash_pw)) {
    return false;
  }
  char storedPassword[PASSWORD_SIZE];
  if (db.getUserPassword(username.c_str(), storedPassword)) {
    return strcmp(hash_pw.c_str(), storedPassword) == 0; 
  }
  return false;
}

bool LoginManager::addLogin(const std::string& username, const std::string& password){
  std::string hashedPassword, d_salt;
  genSalt(d_salt);
  if (d_salt.empty()){
    return false;
  }

  hash(s_salt + password + d_salt,hashedPassword);
  if(hashedPassword.empty()){
    return false;
  }

  if (db.addUser(username.c_str(), hashedPassword.c_str(), d_salt.c_str())) {
    return true;
  }
  return false;
}

bool LoginManager::delLogin(const std::string& username, const std::string& password){
  std::string hash_pw;
  if (!getHashedPassword(username,password,hash_pw)) {
    return false;
  }

  if (db.delUser(username.c_str(), hash_pw.c_str())){
    return true;
  }
  return false;
}

bool LoginManager::getHashedPassword(const std::string& usid, const std::string& pw, std::string& hashed_pw){
  std::string d_salt;
  if (!getSalt(usid, d_salt) || d_salt.empty() ) {
    return false;
  }
  hash(s_salt + pw + d_salt, hashed_pw);
  return !hashed_pw.empty();
}
bool LoginManager::getSalt(const std::string& username, std::string& salt) {
  char storedSalt[SALT_SIZE];  
  if (db.getUserSalt(username.c_str(), storedSalt)) {
    salt = storedSalt;
    return true;
  }
  return false;
}
void LoginManager::genSalt(std::string& salt){
  salt = "56";
}
void LoginManager::hash(const std::string& input, std::string& output) {
    // Dummy hash function that just assigns input to output for demonstration
    output = "hashed_" + input;
}
