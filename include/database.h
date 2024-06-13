#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#define SALT_SIZE 10
#define PASSWORD_SIZE 65

class Database {
public:
  Database(const char* dbFile);
  ~Database();
  bool getUserPassword(const char* secid, char* password);
  bool getUserSalt(const char* secid, char* salt);
  bool addUser(const char* secid, const char* password, const char* salt);
  bool delUser(const char* secid, const char* password);

private:
    sqlite3* db;
    bool selectText(const char* sql, const char* value, char* result, const int size);
};

#endif // DATABASE_H

