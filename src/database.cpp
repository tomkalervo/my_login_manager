#include "database.h"
#include <iostream>

Database::Database(const char* dbFile) {
  if (sqlite3_open(dbFile, &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

Database::~Database() {
  if (db) {
    sqlite3_close(db);
  }
}
bool Database::delUser(const char* secid, const char* password){
  char* errMsg = nullptr;
  int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << errMsg << std::endl;
    sqlite3_free(errMsg);
    return rc;
  }
  
  sqlite3_stmt *stmt;
  const char* select_id= 
    u8"SELECT login.id FROM login "
      "INNER JOIN password ON login.id = password.login_id "
      "WHERE login.secid = ? AND password.password = ?;";
  rc = sqlite3_prepare_v2(db, select_id, -1, &stmt, 0);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
    return rc;
  }

  sqlite3_bind_text(stmt, 1, secid, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
  sqlite3_int64 login_id = -1;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    login_id = sqlite3_column_int64(stmt, 0);
  }
  if (login_id == -1) {
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg); 
    sqlite3_free(errMsg);
    return false;
  }

  sqlite3_finalize(stmt);
  // Delete from password table
  const char* delete_password = "DELETE FROM password WHERE login_id = ?;";
  rc = sqlite3_prepare_v2(db, delete_password, -1, &stmt, 0);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg); 
    sqlite3_free(errMsg);
    return false;
  }

  sqlite3_bind_int64(stmt, 1, login_id);
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
    return false;
  }
  sqlite3_finalize(stmt);
 
  // Delete from login table
  const char* delete_login = "DELETE FROM login WHERE id = ?;";
  rc = sqlite3_prepare_v2(db, delete_login, -1, &stmt, 0);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
    return false;
  }

  sqlite3_bind_int64(stmt, 1, login_id);
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
    return false;
  }
  sqlite3_finalize(stmt); 
  
  rc = sqlite3_exec(db, "COMMIT;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << errMsg << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
    return rc;
  }
  return true;
}

bool Database::addUser(const char* secid, const char* password, const char* salt){
  char* errMsg = nullptr;
  int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
      std::cerr << "SQL error: " << errMsg << std::endl;
      sqlite3_free(errMsg);
      return rc;
  }

  const char *insert_login_query = u8"INSERT INTO login (secid,salt) VALUES (?, ?);";
  sqlite3_stmt *stmt;
  rc = sqlite3_prepare_v2(db, insert_login_query, -1, &stmt, 0);
  if (rc != SQLITE_OK) {
      std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
      sqlite3_free(errMsg);
      return rc;
  }

  sqlite3_bind_text(stmt, 1, secid, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, salt, -1, SQLITE_STATIC);

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_finalize(stmt);
    sqlite3_free(errMsg);
    return rc;
  }

  sqlite3_finalize(stmt);

  sqlite3_int64 login_id = sqlite3_last_insert_rowid(db);
  const char *insert_password_query = "INSERT INTO password (login_id, password) VALUES (?, ?);";
  rc = sqlite3_prepare_v2(db, insert_password_query, -1, &stmt, 0);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    return rc;
  }

  sqlite3_bind_int64(stmt, 1, login_id);
  sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_finalize(stmt);
    return rc;
  }

  sqlite3_finalize(stmt);

  rc = sqlite3_exec(db, "COMMIT;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << errMsg << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
    return rc;
  }

    return true;
}

bool Database::getUserPassword(const char* secid, char* password) {
  const char* query = 
    "SELECT p.password "
    "FROM login l INNER JOIN password p ON l.id = p.login_id "
    "WHERE l.secid = ? ;";
  return selectText(query,secid,password,PASSWORD_SIZE);
}

bool Database::getUserSalt(const char* secid, char* salt) {
  const char* query = "SELECT salt FROM login WHERE secid = ? ;";
  return selectText(query,secid,salt,SALT_SIZE);
}

bool Database::selectText(const char* sql, const char* value, char* result, const int size) {
  sqlite3_stmt* stmt;
  bool success = false;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "Error preparing SQL statement: " << sqlite3_errmsg(db) << std::endl;
    return false;
  }

  if (sqlite3_bind_text(stmt, 1, value, -1, SQLITE_STATIC) != SQLITE_OK) {
    std::cerr << "Error binding value to parameter: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt); 
    return false;
  }

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* text = sqlite3_column_text(stmt, 0);
    if (text) {
      int length = strlen(reinterpret_cast<const char*>(text));
      int actual_length = std::min(length, size - 1);
      strncpy(result, reinterpret_cast<const char*>(text), actual_length);
      result[actual_length] = '\0';
      success = true;
    }
  }

  sqlite3_finalize(stmt);
  return success;
}
