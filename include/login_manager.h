#ifndef LOGIN_MANAGER_H
#define LOGIN_MANAGER_H

#include <string>
#include "database.h"
#include "hash_password.h"

class LoginManager {
public:
  LoginManager(const std::string& dbFile);
  bool login(const std::string& username, const std::string& password);
  bool addLogin(const std::string& username, const std::string& password);
  bool delLogin(const std::string& username, const std::string& password);
private:
  Database db;
  static const std::string s_salt;
  bool getHashedPassword(const std::string& usid, const std::string& pw, std::string& hashed_pw);
  bool getSalt(const std::string& username, std::string& salt);
  void genSalt(std::string& salt);
  void hash(const std::string& input, std::string& output);
};

#endif // LOGIN_MANAGER_H

