/*
 * Written by Tom Karlsson.
 * 2024-07-20
 *
 * Database object that supports the following operations:
 * Insert new user into database,
 * Delete existing user from database,
 * Check if valid user (secid & password),
 * Get salt associated to existing user,
 * Get password associated to existing user.
 *
 * SQLite supports prepared statements. These statements are compiled into
 * SQLite byte code. This Database object creates and compiles these statments
 * at initatilzation. They are then reused at each transatction to the database
 * - without the need to recompile the byte code.
 *
 */

#include "database.h"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <stdexcept>

Database::Database(const char *dbFile) {
  if (sqlite3_open(dbFile, &db)) {
    std::string error_msg = "Can't open database: " + std::string(sqlite3_errmsg(db));
    //std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_close(db);
    db = nullptr;
    throw std::runtime_error(error_msg);
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

    zSql = u8"SELECT p.password FROM login l "
           u8"INNER JOIN password p on l.id = p.login_id "
           u8"WHERE l.secid = :secid;";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &get_password_stmt,
                           nullptr)) {
      std::cerr << "Can't prepare statement for select of password: "
                << sqlite3_errmsg(db) << std::endl;
      get_password_stmt = nullptr;
    }

    zSql = u8"SELECT salt FROM login WHERE secid = :secid ;";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &get_salt_stmt,
                           nullptr)) {
      std::cerr << "Can't prepare statement for select of salt: "
                << sqlite3_errmsg(db) << std::endl;
      get_salt_stmt = nullptr;
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
  if (get_password_stmt) {
    sqlite3_finalize(get_password_stmt);
  }
  if (get_salt_stmt) {
    sqlite3_finalize(get_salt_stmt);
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

  if (exists == 0) {
    return SQLITE_NOTFOUND;
  } else if (exists == 1) {
    return SQLITE_OK;
  } else {
    return SQLITE_ERROR;
  }
}

int Database::deleteUser(const std::string &secid,
                         const std::string &password) {
  /*
   * INPUT: secid and password for the user to be deleted.
   * RETURN: Integer value. 0-200 represent sqlite3 return codes, 500 is
   * internal server error. Function utilize the global instance statements
   * select_id_stmt, delete_login_stmt and delete_password_stmt. New
   * parameters provided to the function are binded to these statements. The
   * statements are reset upon function completion.
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

int Database::getUserSalt(const string &secid, string &salt) {
  if (!get_salt_stmt) {
    std::cerr << "Error get_salt_stmt not initialized" << std::endl;
    return 500;
  }

  int rc = sqlite3_bind_text(
      get_salt_stmt, sqlite3_bind_parameter_index(get_salt_stmt, ":secid"),
      secid.c_str(), secid.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    std::cerr << "Error bind secid statement: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    sqlite3_reset(get_salt_stmt);
    return rc;
  }
  rc = sqlite3_step(get_salt_stmt);
  if (rc != SQLITE_ROW) {
    std::cerr << "SQL Error, unsuccesful select of salt: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    sqlite3_reset(get_salt_stmt);
    return rc;
  }
  const unsigned char *text = sqlite3_column_text(get_salt_stmt, 0);
  if (text) {
    salt.assign(reinterpret_cast<const char *>(text));
  }
  sqlite3_reset(get_salt_stmt);
  return SQLITE_OK;
}

int Database::getUserPassword(const string &secid, string &password) {
  if (!get_password_stmt) {
    std::cerr << "Error get_password_stmt not initialized" << std::endl;
    return 500;
  }

  int rc = sqlite3_bind_text(
      get_password_stmt,
      sqlite3_bind_parameter_index(get_password_stmt, ":secid"), secid.c_str(),
      secid.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    std::cerr << "Error bind secid statement: " << sqlite3_errmsg(db)
              << ", rc: " << rc << std::endl;
    sqlite3_reset(get_password_stmt);
    return rc;
  }
  rc = sqlite3_step(get_password_stmt);
  if (rc != SQLITE_ROW) {
    std::cerr << "SQL Error, unsuccesful select of password: "
              << sqlite3_errmsg(db) << ", rc: " << rc << std::endl;
    sqlite3_reset(get_password_stmt);
    return rc;
  }
  const unsigned char *text = sqlite3_column_text(get_password_stmt, 0);
  if (text) {
    password.assign(reinterpret_cast<const char *>(text));
  }
  sqlite3_reset(get_password_stmt);
  return SQLITE_OK;
}
