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
 * Update password (and salt) using secid
 *
 * SQLite supports prepared statements. These statements are compiled into
 * SQLite byte code. This Database object creates and compiles these statments
 * at initatilzation. They are then reused at each transatction to the database
 * - without the need to recompile the byte code.
 *
 */

#include "database.h"
#include <cstddef>
#include <stdexcept>
#include <string>

using LogLevel = Logger::LogLevel;

/*
 * Constructor needs filepath to an existing sqlite3 database
 */
Database::Database(const char *dbFile) {
  if (sqlite3_open(dbFile, &db)) {
    string text = "Database::Database Can't open database: ";
    text.append(sqlite3_errmsg(db));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_close(db);
    db = nullptr;
    throw std::runtime_error(text);
  }

  if (db) {
    // Prepare satement for checking password
    std::string zSql = u8"SELECT count(*) FROM login lg "
                       "INNER JOIN password pw ON lg.id = pw.login_id "
                       "WHERE lg.secid = :secid AND pw.password = :password;";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(),
                           &check_password_stmt, nullptr)) {
      string text = "Database::Database Prepare check_password_stmt: ";
      text.append(sqlite3_errmsg(db));
      m_log->entry(LogLevel::ERROR, text);
      check_password_stmt = nullptr;
    }

    // Prepare satements for delete login data
    zSql = u8"SELECT lg.id FROM login lg "
           "INNER JOIN password pw ON lg.id = pw.login_id "
           "WHERE lg.secid = :secid AND pw.password = :password;";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &select_id_stmt,
                           nullptr)) {
      string text = "Database::Database Prepare select_id_stmt: ";
      text.append(sqlite3_errmsg(db));
      m_log->entry(LogLevel::ERROR, text);
      select_id_stmt = nullptr;
    }

    zSql = u8"DELETE FROM login WHERE id = :id;";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &delete_login_stmt,
                           nullptr)) {
      string text = "Database::Database Prepare delete_login_stmt: ";
      text.append(sqlite3_errmsg(db));
      m_log->entry(LogLevel::ERROR, text);
      delete_login_stmt = nullptr;
    }

    zSql = u8"DELETE FROM password WHERE login_id = :id;";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(),
                           &delete_password_stmt, nullptr)) {
      string text = "Database::Database Prepare delete_password_stmt: ";
      text.append(sqlite3_errmsg(db));
      m_log->entry(LogLevel::ERROR, text);
      delete_password_stmt = nullptr;
    }

    zSql = u8"INSERT INTO login (secid, salt) VALUES (:secid, :salt);";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &add_login_stmt,
                           nullptr)) {
      string text = "Database::Database Prepare add_login_stmt: ";
      text.append(sqlite3_errmsg(db));
      m_log->entry(LogLevel::ERROR, text);
      add_login_stmt = nullptr;
    }

    zSql = u8"INSERT INTO password (login_id, password) VALUES (:login_id, "
           u8":password);";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &add_password_stmt,
                           nullptr)) {
      string text = "Database::Database Prepare add_password_stmt: ";
      text.append(sqlite3_errmsg(db));
      m_log->entry(LogLevel::ERROR, text);
      add_password_stmt = nullptr;
    }

    zSql = u8"SELECT p.password FROM login l "
           u8"INNER JOIN password p on l.id = p.login_id "
           u8"WHERE l.secid = :secid;";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &get_password_stmt,
                           nullptr)) {
      string text = "Database::Database Prepare get_password_stmt: ";
      text.append(sqlite3_errmsg(db));
      m_log->entry(LogLevel::ERROR, text);
      get_password_stmt = nullptr;
    }

    zSql = u8"SELECT salt FROM login WHERE secid = :secid ;";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &get_salt_stmt,
                           nullptr)) {
      string text = "Database::Database Prepare get_salt_stmt: ";
      text.append(sqlite3_errmsg(db));
      m_log->entry(LogLevel::ERROR, text);
      get_salt_stmt = nullptr;
    }

    zSql = u8"UPDATE login SET salt = :salt "
           u8"WHERE secid = :secid;";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &upd_salt_stmt,
                           nullptr)) {
      string text = "Database::Database Prepare upd_salt_stmt: ";
      text.append(sqlite3_errmsg(db));
      m_log->entry(LogLevel::ERROR, text);
      upd_salt_stmt = nullptr;
    }

    zSql = u8"UPDATE password SET password = :password "
           u8"WHERE login_id = (SELECT id FROM login WHERE secid = :secid);";

    if (sqlite3_prepare_v2(db, zSql.c_str(), zSql.length(), &upd_password_stmt,
                           nullptr)) {
      string text = "Database::Database Prepare upd_password_stmt: ";
      text.append(sqlite3_errmsg(db));
      m_log->entry(LogLevel::ERROR, text);
      upd_password_stmt = nullptr;
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
  if (upd_salt_stmt) {
    sqlite3_finalize(upd_salt_stmt);
  }
  if (upd_password_stmt) {
    sqlite3_finalize(upd_password_stmt);
  }
  if (db) {
    sqlite3_close(db);
  }
}

void Database::setLogger(Logger *log) {
  if (log) {
    m_log = log;
    m_log->entry(LogLevel::INFO,
                 "Database::setLogger Logger added to Database object.");
  }
}

int Database::checkPassword(const std::string &secid,
                            const std::string &password) {
  if (!check_password_stmt) {
    m_log->entry(LogLevel::ERROR,
                 "Database::checkPassword check_password_stmt not initialized");
    return SQLITE_ERROR;
  }

  int rc = sqlite3_bind_text(
      check_password_stmt,
      sqlite3_bind_parameter_index(check_password_stmt, ":secid"),
      secid.c_str(), secid.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    string text = "Database::checkPassword statement bind 'secid': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    return rc;
  }

  rc = sqlite3_bind_text(
      check_password_stmt,
      sqlite3_bind_parameter_index(check_password_stmt, ":password"),
      password.c_str(), password.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    string text = "Database::checkPassword statement bind 'password': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    return rc;
  }

  rc = sqlite3_step(check_password_stmt);
  if (rc != SQLITE_ROW) {
    string text = "Database::checkPassword execute step check_password_stmt: ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
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
    string text = "Database::checkPassword final result returned: ";
    text.append(std::to_string(exists));
    text.append(" columns for 'secid': ");
    text.append(secid);
    m_log->entry(LogLevel::WARNING, text);
    return SQLITE_WARNING;
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
    m_log->entry(LogLevel::ERROR,
                 "Database::deleteUser select_id_stmt not initialized");
    return SQLITE_ERROR;
  }

  if (!delete_login_stmt) {
    m_log->entry(LogLevel::ERROR,
                 "Database::deleteUser delete_login_stmt not initialized");
    return SQLITE_ERROR;
  }

  if (!delete_password_stmt) {
    m_log->entry(LogLevel::ERROR,
                 "Database::deleteUser delete_password_stmt not initialized");
    return SQLITE_ERROR;
  }

  int rc = sqlite3_bind_text(
      select_id_stmt, sqlite3_bind_parameter_index(select_id_stmt, ":secid"),
      secid.c_str(), secid.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    string text = "Database::deleteUser bind statement 'secid': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_reset(select_id_stmt);
    return rc;
  }

  rc = sqlite3_bind_text(
      select_id_stmt, sqlite3_bind_parameter_index(select_id_stmt, ":password"),
      password.c_str(), password.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    string text = "Database::deleteUser bind statement 'password': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_reset(select_id_stmt);
    return rc;
  }

  char *errMsg = nullptr;
  rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    string text = "Database::deleteUser execute BEGIN: ";
    text.append(errMsg);
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_free(errMsg);
    sqlite3_reset(select_id_stmt);
    return rc;
  }

  rc = sqlite3_step(select_id_stmt);
  if (rc != SQLITE_ROW) {
    string text = "Database::deleteUser execute step select id: ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    text.append(" >> ROLLBACK");

    if (rc == SQLITE_DONE) {
      m_log->entry(LogLevel::INFO, text);
    } else {
      m_log->entry(LogLevel::ERROR, text);
    }
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Database::deleteUser exevute step select id following ROLLBACK: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
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
    string text = "Database::deleteUser bind delete login statement 'id': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    text.append(" >> ROLLBACK");
    m_log->entry(LogLevel::ERROR, text);
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Following ROLLBACK: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
    sqlite3_free(errMsg);
    sqlite3_reset(delete_login_stmt);
    return rc;
  }

  rc = sqlite3_step(delete_login_stmt);
  sqlite3_reset(delete_login_stmt);
  if (rc != SQLITE_DONE) {
    string text = "Database::deleteUser execute step delete login: ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    text.append(" >> ROLLBACK");
    m_log->entry(LogLevel::ERROR, text);
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Following ROLLBACK: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
    sqlite3_free(errMsg);
    return rc;
  }

  // Delete from table password
  rc = sqlite3_bind_int(
      delete_password_stmt,
      sqlite3_bind_parameter_index(delete_password_stmt, ":id"), id);
  if (rc != SQLITE_OK) {
    string text = "Database::deleteUser bind delete password statement 'id': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    text.append(" >> ROLLBACK");
    m_log->entry(LogLevel::ERROR, text);
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Following ROLLBACK: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
    sqlite3_free(errMsg);
    sqlite3_reset(delete_password_stmt);
    return rc;
  }

  rc = sqlite3_step(delete_password_stmt);
  sqlite3_reset(delete_password_stmt);
  if (rc != SQLITE_DONE) {
    string text = "Database::deleteUser execute step delete password: ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    text.append(" >> ROLLBACK");
    m_log->entry(LogLevel::ERROR, text);
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Following ROLLBACK: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
    sqlite3_free(errMsg);
    return rc;
  }

  rc = sqlite3_exec(db, "COMMIT;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    string text = "Database::deleteUser execute step COMMIT: ";
    text.append(errMsg);
    text.append(", rc: ");
    text.append(std::to_string(rc));
    text.append(" >> ROLLBACK");
    m_log->entry(LogLevel::ERROR, text);
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Following ROLLBACK: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
    sqlite3_free(errMsg);
    return rc;
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
    m_log->entry(LogLevel::ERROR,
                 "Database::addUser add_login_stmt not initialized");
    return SQLITE_ERROR;
  }

  if (!add_password_stmt) {
    m_log->entry(LogLevel::ERROR,
                 "Database::addUser add_password_stmt not initialized");
    return SQLITE_ERROR;
  }

  int rc = sqlite3_bind_text(
      add_login_stmt, sqlite3_bind_parameter_index(add_login_stmt, ":secid"),
      secid.c_str(), secid.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    string text = "Database::addUser bind statement 'secid': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_reset(add_login_stmt);
    return rc;
  }

  rc = sqlite3_bind_text(add_login_stmt,
                         sqlite3_bind_parameter_index(add_login_stmt, ":salt"),
                         salt.c_str(), salt.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    string text = "Database::addUser bind statement 'salt': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_reset(add_login_stmt);
    return rc;
  }
  char *errMsg = nullptr;
  rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    string text = "Database::addUser BEGIN IMMEDIATE: ";
    text.append(errMsg);
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_free(errMsg);
    sqlite3_reset(add_login_stmt);
    return rc;
  }

  rc = sqlite3_step(add_login_stmt);
  sqlite3_reset(add_login_stmt);
  if (rc != SQLITE_DONE) {
    string text = "Database::addUser execute step add_login_stmt: ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    text.append(" >> ROLLBACK");
    m_log->entry(LogLevel::ERROR, text);
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Database::addUser execute step select id following ROLLBACK: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
    sqlite3_free(errMsg);
    return rc;
  }

  // Bind password statement
  sqlite3_int64 login_id = sqlite3_last_insert_rowid(db);

  rc = sqlite3_bind_int64(
      add_password_stmt,
      sqlite3_bind_parameter_index(add_password_stmt, ":login_id"), login_id);
  if (rc != SQLITE_OK) {
    string text = "Database::addUser bind statement 'login_id': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_reset(add_password_stmt);
    return rc;
  }

  rc = sqlite3_bind_text(
      add_password_stmt,
      sqlite3_bind_parameter_index(add_password_stmt, ":password"),
      password.c_str(), password.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    string text = "Database::addUser bind statement 'password': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_reset(add_password_stmt);
    return rc;
  }

  rc = sqlite3_step(add_password_stmt);
  sqlite3_reset(add_password_stmt);
  if (rc != SQLITE_DONE) {
    string text = "Database::addUser execute step add_password_stmt: ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    text.append(" >> ROLLBACK");
    m_log->entry(LogLevel::ERROR, text);
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Database::addUser execute step add_password_stmt following "
             "ROLLBACK: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
    sqlite3_free(errMsg);
    return rc;
  }

  rc = sqlite3_exec(db, "COMMIT;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    string text = "Database::addUser execute step COMMIT: ";
    text.append(errMsg);
    text.append(", rc: ");
    text.append(std::to_string(rc));
    text.append(" >> ROLLBACK");
    m_log->entry(LogLevel::ERROR, text);
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Following ROLLBACK: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
    sqlite3_free(errMsg);
  }

  return rc;
}

int Database::updatePassword(const string &secid, const string &password,
                             const string &salt) {
  /*
   * INPUT: secid for existing user, password and generated salt to be updated.
   * RETURN: Integer value. 0-200 represent sqlite3 return codes, 500 is
   * internal server error.
   */
  if (!upd_salt_stmt) {
    m_log->entry(LogLevel::ERROR,
                 "Database::updatePassword upd_salt_stmt not initialized");
    return SQLITE_ERROR;
  }

  if (!upd_password_stmt) {
    m_log->entry(LogLevel::ERROR,
                 "Database::updatePassword upd_password_stmt not initialized");
    return SQLITE_ERROR;
  }

  // Bind parameters to SQL-queries
  int rc = sqlite3_bind_text(
      upd_salt_stmt, sqlite3_bind_parameter_index(upd_salt_stmt, ":secid"),
      secid.c_str(), secid.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    string text = "Database::updatePassword bind upd_salt_stmt w/ 'secid': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_reset(upd_salt_stmt);
    return rc;
  }

  rc = sqlite3_bind_text(upd_salt_stmt,
                         sqlite3_bind_parameter_index(upd_salt_stmt, ":salt"),
                         salt.c_str(), salt.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    string text = "Database::updatePassword bind upd 'salt': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_reset(upd_salt_stmt);
    return rc;
  }

  rc = sqlite3_bind_text(
      upd_password_stmt,
      sqlite3_bind_parameter_index(upd_password_stmt, ":secid"), secid.c_str(),
      secid.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    string text =
        "Database::updatePassword bind upd_password_stmt w/ 'secid': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_reset(upd_password_stmt);
    return rc;
  }

  rc = sqlite3_bind_text(
      upd_password_stmt,
      sqlite3_bind_parameter_index(upd_password_stmt, ":password"),
      password.c_str(), password.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    string text = "Database::updatePassword bind upd_salt_stmt w/ 'password': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_reset(upd_password_stmt);
    return rc;
  }

  // Start database transatction
  char *errMsg = nullptr;
  rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    string text = "Database::updatePassword BEGIN IMMEDIATE: ";
    text.append(errMsg);
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_free(errMsg);
    sqlite3_reset(upd_salt_stmt);
    sqlite3_reset(upd_password_stmt);
    return rc;
  }
  // Execute the first update statement
  rc = sqlite3_step(upd_salt_stmt);
  sqlite3_reset(upd_salt_stmt);
  if (rc != SQLITE_DONE) {
    string text = "Database::updatePassword execute step upd_salt_stmt: ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    text.append(" >> ROLLBACK");
    m_log->entry(LogLevel::ERROR, text);
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Database::updatePassword execute step upd_salt_stmt following "
             "ROLLBACK: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
    sqlite3_free(errMsg);
    return rc;
  }

  // Check the row count, abort if no unique secid
  int rowsAffected = sqlite3_changes(db);
  if (rowsAffected != 1) {
    string text = "Database::updatePassword Affected Rows in upd_salt_stmt not "
                  "1, actual: ";
    text.append(std::to_string(rowsAffected));
    m_log->entry(LogLevel::INFO, text);
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Database::updatePassword ROLLBACK after invalid rowsAffected: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
    sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg);
    sqlite3_reset(upd_password_stmt);
    return SQLITE_ABORT;
  }

  // Execute the second update statement
  rc = sqlite3_step(upd_password_stmt);
  sqlite3_reset(upd_password_stmt);
  if (rc != SQLITE_DONE) {
    string text = "Database::updatePassword execute step upd_password_stmt: ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    text.append(" >> ROLLBACK");
    m_log->entry(LogLevel::ERROR, text);
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Database::updatePassword execute step upd_password_stmt "
             "following ROLLBACK: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
    sqlite3_free(errMsg);
    return rc;
  }

  // Check the row count, abort if no unique secid
  rowsAffected = sqlite3_changes(db);
  if (rowsAffected != 1) {
    string text =
        "Database::updatePassword Affected Rows in upd_password_stmt not "
        "1, actual: ";
    text.append(std::to_string(rowsAffected));
    m_log->entry(LogLevel::INFO, text);
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Database::updatePassword ROLLBACK after invalid rowsAffected: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
    sqlite3_free(errMsg);
    return SQLITE_ABORT;
  }

  rc = sqlite3_exec(db, "COMMIT;", 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    string text = "Database::updatePassword execute step COMMIT: ";
    text.append(errMsg);
    text.append(", rc: ");
    text.append(std::to_string(rc));
    text.append(" >> ROLLBACK");
    m_log->entry(LogLevel::ERROR, text);
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, &errMsg) != SQLITE_OK) {
      text = "Following ROLLBACK: ";
      text.append(errMsg);
      m_log->entry(LogLevel::ERROR, text);
    }
    sqlite3_free(errMsg);
  }

  return rc;
}
int Database::getUserSalt(const string &secid, string &salt) {
  if (!get_salt_stmt) {
    m_log->entry(LogLevel::ERROR,
                 "Database::getUserSalt get_salt_stmt not initialized");
    return SQLITE_ERROR;
  }

  int rc = sqlite3_bind_text(
      get_salt_stmt, sqlite3_bind_parameter_index(get_salt_stmt, ":secid"),
      secid.c_str(), secid.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    string text = "Database::getUserSalt bind get_salt_stmt w/ 'secid': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_reset(get_salt_stmt);
    return rc;
  }
  rc = sqlite3_step(get_salt_stmt);
  if (rc != SQLITE_ROW) {
    string text = "Database::getUserSalt execute step get_salt_stmt: ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::INFO, text);
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
    m_log->entry(LogLevel::ERROR,
                 "Database::getUserPassword get_password_stmt not initialized");
    return SQLITE_ERROR;
  }

  int rc = sqlite3_bind_text(
      get_password_stmt,
      sqlite3_bind_parameter_index(get_password_stmt, ":secid"), secid.c_str(),
      secid.length(), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    string text =
        "Database::getUserPassword bind get_password_stmt w/ 'secid': ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::ERROR, text);
    sqlite3_reset(get_password_stmt);
    return rc;
  }
  rc = sqlite3_step(get_password_stmt);
  if (rc != SQLITE_ROW) {
    string text = "Database::getUserPassword execute step get_password_stmt: ";
    text.append(sqlite3_errmsg(db));
    text.append(", rc: ");
    text.append(std::to_string(rc));
    m_log->entry(LogLevel::INFO, text);
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
