#ifndef LOGIN_MANAGER_H
#define LOGIN_MANAGER_H

#include "database.h"
#include <random>
#include <string>

class LoginManager {
public:
  LoginManager(const std::string &dbFile);
  void startAPI();
  void stopAPI();
  int login(const std::string &username, const std::string &password);
  int addLogin(const std::string &username, const std::string &password);
  int delLogin(const std::string &username, const std::string &password);

private:
  Database db;
  static const std::string STATIC_SALT;
  std::mt19937 salt_generator;
  void *api_status;
  bool getHashedPassword(const std::string &usid, const std::string &pw,
                         std::string &hashed_pw);
  bool getSalt(const std::string &username, std::string &salt);
  std::string generateSalt();
  void hash(const std::string &input, std::string &output);
};

#endif // LOGIN_MANAGER_H
