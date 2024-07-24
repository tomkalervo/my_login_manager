#include "database.h"
#include <cstddef>
#include <iostream>

Database::Database(const char *dbFile) {
  if (sqlite3_open(dbFile, &db)) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    db = nullptr;
  }
  if (db) {
    // Prepare satement for checking password
    std::string zSql = u8"SELECT count(*) FROM login lg "
                       "INNER JOIN password pw ON lg.id = pw.login_id "
                       "WHERE lg.secid = :secid AND pw.password = :password;";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(),
                           &check_password_stmt, nullptr)) {
      std::cerr << "Can't prepare statement for check password: "
                << sqlite3_errmsg(db) << std::endl;
      check_password_stmt = nullptr;
    }

    // Prepare satements for delete login data
    zSql = u8"SELECT lg.id FROM login lg "
           "INNER JOIN password pw ON lg.id = pw.login_id "
           "WHERE lg.secid = :secid AND pw.password = :password;";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &select_id_stmt,
                           nullptr)) {
      std::cerr << "Can't prepare statement for select of login id: "
                << sqlite3_errmsg(db) << std::endl;
      select_id_stmt = nullptr;
    }

    zSql = u8"DELETE FROM login WHERE id = :id;";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &delete_login_stmt,
                           nullptr)) {
      std::cerr << "Can't prepare statement for delete of login data: "
                << sqlite3_errmsg(db) << std::endl;
      delete_login_stmt = nullptr;
    }

    zSql = u8"DELETE FROM password WHERE login_id = :id;";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(),
                           &delete_password_stmt, nullptr)) {
      std::cerr << "Can't prepare statement for delete of password data: "
                << sqlite3_errmsg(db) << std::endl;
      delete_password_stmt = nullptr;
    }

    zSql = u8"INSERT INTO login (secid, salt) VALUES (:secid, :salt);";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &add_login_stmt,
                           nullptr)) {
      std::cerr << "Can't prepare statement for insert of login data: "
                << sqlite3_errmsg(db) << std::endl;
      add_login_stmt = nullptr;
    }

    zSql = u8"INSERT INTO password (login_id, password) VALUES (:login_id, "
           u8":password);";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &add_password_stmt,
                           nullptr)) {
      std::cerr << "Can't prepare statement for insert of password data: "
                << sqlite3_errmsg(db) << std::endl;
      add_password_stmt = nullptr;
    }
  }
}

Database::~Database() {
  if (check_password_stmt) {
    sqlite3_finalize(check_password_stmt);
  }
  if (select_id_stmt) {
    sqlite3_finalize(select_id_stmt);
  }
  if (delete_login_stmt) {
    sqlite3_finalize(delete_login_stmt);
  }
  if (delete_password_stmt) {
    sqlite3_finalize(delete_password_stmt);
  }
  if (add_login_stmt) {
    sqlite3_finalize(add_login_stmt);
  }
  if (add_password_stmt) {
    sqlite3_finalize(add_password_stmt);
  }
  if (db) {
    sqlite3_close(db);
  }
}

int Database::checkPassword(const std::string &secid,
                            const std::string &password) {
  if (!check_password_stmt) {
    std::cerr << "Error check_password_stmt not initialized" << std::endl;
    return 500;
  }

  int rc = sqlite3_bind_text(
      check_password_stmt,
      sqlite3_bind_parameter_index(check_password_stmt, ":secid"),
      secid.c_str(), secid.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    std::cerr << "Error bind secid statement: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    return rc;
  }

  rc = sqlite3_bind_text(
      check_password_stmt,
      sqlite3_bind_parameter_index(check_password_stmt, ":password"),
      password.c_str(), password.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    std::cerr << "Error bind password statement: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    return rc;
  }

  rc = sqlite3_step(check_password_stmt);
  if (rc != SQLITE_ROW) {
    std::cerr << "Error running sql-step: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    sqlite3_reset(check_password_stmt);
    return rc;
  }

  int exists = sqlite3_column_int(check_password_stmt, 0);
  sqlite3_reset(check_password_stmt);

  return 0 - exists;
}

int Database::deleteUser(const std::string &secid,
                         const std::string &password) {
  /*
   * INPUT: secid and password for the user to be deleted.
   * RETURN: Integer value. 0-200 represent sqlite3 return codes, 500 is
   * internal server error. Function utilize the global instance statements
   * select_id_stmt, delete_login_stmt and delete_password_stmt. New parameters
   * provided to the function are binded to these statements. The statements are
   * reset upon function completion.
   */
  if (!select_id_stmt) {
    std::cerr << "Error select_id_stmt not initialized" << std::endl;
    return 500;
  }

  if (!delete_login_stmt) {
    std::cerr << "Error delete_login_stmt not initialized" << std::endl;
    return 500;
  }

  if (!delete_password_stmt) {
    std::cerr << "Error delete_password_stmt not initialized" << std::endl;
    return 500;
  }

  int rc = sqlite3_bind_text(
      select_id_stmt, sqlite3_bind_parameter_index(select_id_stmt, ":secid"),
      secid.c_str(), secid.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    std::cerr << "Error bind secid statement: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    sqlite3_reset(select_id_stmt);
    return rc;
  }

  rc = sqlite3_bind_text(
      select_id_stmt, sqlite3_bind_parameter_index(select_id_stmt, ":password"),
      password.c_str(), password.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    std::cerr << "Error bind password statement: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    sqlite3_reset(select_id_stmt);
    return rc;
  }

  char *errMsg = nullptr;
  rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error at begin immediate: " << errMsg << std::endl;
    sqlite3_free(errMsg);
    sqlite3_reset(select_id_stmt);
    return rc;
  }

  rc = sqlite3_step(select_id_stmt);
  if (rc != SQLITE_ROW) {
    std::cerr << "SQL error at select_id_stmt: " << sqlite3_errmsg(db)
              << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
    sqlite3_reset(select_id_stmt);
    return rc;
  }

  sqlite3_int64 id = sqlite3_column_int(select_id_stmt, 0);
  sqlite3_reset(select_id_stmt);

  // Delete from table login
  rc = sqlite3_bind_int(delete_login_stmt,
                        sqlite3_bind_parameter_index(delete_login_stmt, ":id"),
                        id);
  if (rc != SQLITE_OK) {
    std::cerr << "Error bind login id statement: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
    sqlite3_reset(delete_login_stmt);
    return rc;
  }

  rc = sqlite3_step(delete_login_stmt);
  sqlite3_reset(delete_login_stmt);
  if (rc != SQLITE_DONE) {
    std::cerr << "SQL error at delete_login_stmt: " << sqlite3_errmsg(db)
              << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
    return rc;
  }

  // Delete from table password
  rc = sqlite3_bind_int(
      delete_password_stmt,
      sqlite3_bind_parameter_index(delete_password_stmt, ":id"), id);
  if (rc != SQLITE_OK) {
    std::cerr << "Error bind password id statement: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
    sqlite3_reset(delete_password_stmt);
    return rc;
  }

  rc = sqlite3_step(delete_password_stmt);
  sqlite3_reset(delete_password_stmt);
  if (rc != SQLITE_DONE) {
    std::cerr << "SQL error at delete_password_stmt: " << sqlite3_errmsg(db)
              << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
    return rc;
  }

  rc = sqlite3_exec(db, "COMMIT;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error at commit: " << errMsg << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
  }

  return rc;
}

int Database::addUser(const string &secid, const string &password,
                      const string &salt) {
  /*
   * INPUT: secid, password and generated salt for the user to be added.
   * RETURN: Integer value. 0-200 represent sqlite3 return codes, 500 is
   * internal server error.
   */
  if (!add_login_stmt) {
    std::cerr << "Error add_login_stmt not initialized" << std::endl;
    return 500;
  }

  if (!add_password_stmt) {
    std::cerr << "Error add_password_stmt not initialized" << std::endl;
    return 500;
  }

  int rc = sqlite3_bind_text(
      add_login_stmt, sqlite3_bind_parameter_index(add_login_stmt, ":secid"),
      secid.c_str(), secid.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    std::cerr << "Error bind secid statement: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    sqlite3_reset(add_login_stmt);
    return rc;
  }

  rc = sqlite3_bind_text(add_login_stmt,
                         sqlite3_bind_parameter_index(add_login_stmt, ":salt"),
                         salt.c_str(), salt.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    std::cerr << "Error bind salt statement: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    sqlite3_reset(add_login_stmt);
    return rc;
  }
  char *errMsg = nullptr;
  rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error at begin immediate: " << errMsg << std::endl;
    sqlite3_free(errMsg);
    sqlite3_reset(add_login_stmt);
    return rc;
  }

  rc = sqlite3_step(add_login_stmt);
  sqlite3_reset(add_login_stmt);
  if (rc != SQLITE_DONE) {
    std::cerr << "SQL error at add_login_stmt: " << sqlite3_errmsg(db)
              << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
    return rc;
  }

  // Bind password statement
  sqlite3_int64 login_id = sqlite3_last_insert_rowid(db);

  rc = sqlite3_bind_int64(
      add_password_stmt,
      sqlite3_bind_parameter_index(add_password_stmt, ":login_id"), login_id);
  if (rc != SQLITE_OK) {
    std::cerr << "Error bind password statement: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    sqlite3_reset(add_password_stmt);
    return rc;
  }

  rc = sqlite3_bind_text(
      add_password_stmt,
      sqlite3_bind_parameter_index(add_password_stmt, ":password"),
      password.c_str(), password.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    std::cerr << "Error bind password statement: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    sqlite3_reset(add_password_stmt);
    return rc;
  }

  rc = sqlite3_step(add_password_stmt);
  sqlite3_reset(add_password_stmt);
  if (rc != SQLITE_DONE) {
    std::cerr << "SQL error at add_password_stmt: " << sqlite3_errmsg(db)
              << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
    return rc;
  }

  rc = sqlite3_exec(db, "COMMIT;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error at commit: " << errMsg << std::endl;
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_free(errMsg);
  }

  return rc;
}

bool Database::getUserPassword(const char *secid, char *password) {
  const char *query = "SELECT p.password "
                      "FROM login l INNER JOIN password p ON l.id = p.login_id "
                      "WHERE l.secid = ? ;";
  return selectText(query, secid, password, PASSWORD_SIZE);
}

bool Database::getUserSalt(const char *secid, char *salt) {
  const char *query = "SELECT salt FROM login WHERE secid = ? ;";
  return selectText(query, secid, salt, SALT_SIZE);
}

bool Database::selectText(const char *sql, const char *value, char *result,
                          const int size) {
  sqlite3_stmt *stmt;
  bool success = false;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "Error preparing SQL statement: " << sqlite3_errmsg(db)
              << std::endl;
    return false;
  }

  if (sqlite3_bind_text(stmt, 1, value, -1, SQLITE_STATIC) != SQLITE_OK) {
    std::cerr << "Error binding value to parameter: " << sqlite3_errmsg(db)
              << std::endl;
    sqlite3_finalize(stmt);
    return false;
  }

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char *text = sqlite3_column_text(stmt, 0);
    if (text) {
      int length = strlen(reinterpret_cast<const char *>(text));
      int actual_length = std::min(length, size - 1);
      strncpy(result, reinterpret_cast<const char *>(text), actual_length);
      result[actual_length] = '\0';
      success = true;
    }
  }

  sqlite3_finalize(stmt);
  return success;
}
