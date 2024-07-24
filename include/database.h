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
  int getUserPassword(const string &secid, string &password);
  int getUserSalt(const string &secid, string &salt);
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
  sqlite3_stmt *get_password_stmt;
  sqlite3_stmt *get_salt_stmt;
};

#endif // DATABASE_H
