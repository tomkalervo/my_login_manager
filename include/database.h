#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
using std::string;
#define SALT_SIZE 10
#define PASSWORD_SIZE 65

class Database {
public:
  Database(const char *dbFile);
  ~Database();
  bool getUserPassword(const char *secid, char *password);
  bool getUserSalt(const char *secid, char *salt);
  int addUser(const string &secid, const string &password, const string &salt);
  int deleteUser(const string &secid, const string &password);
  int checkPassword(const string &secid, const string &password);

private:
  sqlite3 *db;
  sqlite3_stmt *check_password_stmt;
  sqlite3_stmt *select_id_stmt;
  sqlite3_stmt *delete_login_stmt;
  sqlite3_stmt *delete_password_stmt;
  sqlite3_stmt *add_login_stmt;
  sqlite3_stmt *add_password_stmt;
  bool selectText(const char *sql, const char *value, char *result,
                  const int size);
};

#endif // DATABASE_H
