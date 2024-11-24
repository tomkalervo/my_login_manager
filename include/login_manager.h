#ifndef LOGIN_MANAGER_H
#define LOGIN_MANAGER_H

#include "database.h"
#include "logger.h"
#include <random>
#include <string>

class LoginManager {
public:
  LoginManager(const std::string &dbFile);
  void logToFile(string const &fpath);
  void setLogLevel(Logger::LogLevel const &level);
  void logToStdout();
  void startAPI();
  void stopAPI();
  int login(const std::string &username, const std::string &password);
  int addLogin(const std::string &username, const std::string &password);
  int delLogin(const std::string &username, const std::string &password);
  int changePassword(const std::string &username, const std::string &password);

private:
  Database m_db;
  std::string const STATIC_SALT = "42";
  std::mt19937 m_salt_generator;
  Logger m_log;
  void *pm_api_status;
  bool getHashedPassword(const std::string &usid, const std::string &pw,
                         std::string &hashed_pw);
  bool getSalt(const std::string &username, std::string &salt);
  std::string generateSalt();
  void hash(const std::string &input, std::string &output);
};

#endif // LOGIN_MANAGER_H
