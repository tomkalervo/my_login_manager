#include "login_manager.h"
#include "database.h"
#include "hash_password.h"
#include "udp_server.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

using std::string;
using LogLevel = Logger::LogLevel;
using LogOut = Logger::LogOut;
/*
 * Initialize LoginManager with the path to the database (login.db)
 */
LoginManager::LoginManager(const string &dbFile) try
    : m_db(dbFile.c_str()), m_log(LogLevel::ERROR, LogOut::STDOUT) {
  m_db.setLogger(&m_log);
  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  m_salt_generator.seed(seed);
} catch (const std::runtime_error &e) {
  std::cerr << "LoginManager::LoginManager Failed to initialize database: "
            << e.what() << std::endl;
  throw;
}
/*
 * Methods for Logger settings
 */
void LoginManager::logToFile(string const &fpath) { m_log.outFilePath(fpath); }
void LoginManager::setLogLevel(LogLevel const &level) { m_log.level(level); }

// TODO
void LoginManager::logToStdout() {}

/*
 * Methods for server control
 */
void LoginManager::startAPI() { pm_api_status = udpServer::run(*this); }
void LoginManager::stopAPI() {
  int rc = udpServer::stop(pm_api_status);
  if (rc) {
    m_log.entry(LogLevel::INFO,
                "LoginManager::stopAPI Server closed successfully.");
  } else {
    m_log.entry(LogLevel::ERROR,
                "LoginManager::stopAPI Server failed to close.");
  }
}
/*
 * Class methods for managing database interaction
 */
int LoginManager::login(const string &username, const string &password) {
  string hash_pw;
  if (!getHashedPassword(username, password, hash_pw)) {
    string text =
        "LoginManager::login Could not get hashed password for username: " +
        username;
    m_log.entry(LogLevel::INFO, text);
    return -1;
  }
  return m_db.checkPassword(username, hash_pw);
}

int LoginManager::addLogin(const string &username, const string &password) {
  string d_salt = generateSalt();
  if (d_salt.empty()) {
    return -1;
  }

  string hashedPassword =
      HashPassword::usingSHA256(STATIC_SALT + password + d_salt);
  if (hashedPassword.empty()) {
    return -2;
  }
  return m_db.addUser(username, hashedPassword, d_salt);
}
int LoginManager::delLogin(const string &username, const string &password) {
  string hash_pw;
  if (!getHashedPassword(username, password, hash_pw)) {
    return -1;
  }
  return m_db.deleteUser(username, hash_pw);
}

int LoginManager::changePassword(const string &username,
                                 const string &password) {
  string d_salt = generateSalt();
  if (d_salt.empty()) {
    return -1;
  }

  string hash_pw = HashPassword::usingSHA256(STATIC_SALT + password + d_salt);
  if (hash_pw.empty()) {
    return -2;
  }
  return m_db.updatePassword(username, hash_pw, d_salt);
}

/*
 * Helper-functions defined below.
 */
bool LoginManager::getHashedPassword(const string &usid, const string &pw,
                                     string &hashed_pw) {
  string d_salt;
  if (!getSalt(usid, d_salt) || d_salt.empty()) {
    string text =
        "LoginManager::getHashedPassword Could not get salt with usid: " + usid;
    m_log.entry(LogLevel::WARNING, text);
    return false;
  }
  hashed_pw = HashPassword::usingSHA256(STATIC_SALT + pw + d_salt);
  return !hashed_pw.empty();
}
bool LoginManager::getSalt(const string &username, string &salt) {
  return (m_db.getUserSalt(username, salt) == 0);
}
string LoginManager::generateSalt() {
  std::stringstream stream;
  stream << std::hex << m_salt_generator();
  return string(stream.str());
}
