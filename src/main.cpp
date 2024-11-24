#include "login_manager.h"
#include <cstring>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <yaml-cpp/yaml.h>

void print_usage() {
  std::cout
      << "./login_manager [-sp path_to_settings] | [-dp path_to_database] \n\n";

  std::cout << "Options:\n";
  std::cout << "  -sp  Path to the settings file.\n";
  std::cout << "  -dp  Path to the database file.\n\n";

  std::cout
      << "Note: You must provide either the path to a settings file with `-sp`";
  std::cout << " or a path to a database file with `-dp`, but not both."
            << std::endl;
}
void print_commands(bool const &server) {
  std::cout << "Following commands are available: \n";
  std::cout << "l    - login \n";
  std::cout << "a    - add new user \n";
  std::cout << "d    - delete existing user \n";
  std::cout << "c    - change password for an existing user \n";
  std::cout << "s    - starts or stops the server \n";
  if (server) {
    std::cout << "       > Server is running, s will stop.\n";
  } else {
    std::cout << "       > Server is not running, s will start.\n";
  }
  std::cout << "q    - quit " << std::endl;
}
void event_loop(LoginManager *lm) {
  bool server_running = false;
  std::string command;

  while (command != "q") {
    std::cout << "Enter command (h for help): ";
    std::cin >> command;
    if (command == "h") {
      print_commands(server_running);
    } else if (command == "l") {
      std::string username, password;

      std::cout << "Enter username: ";
      std::cin >> username;

      std::cout << "Enter password: ";
      std::cin >> password;

      if (0 == lm->login(username, password)) {
        std::cout << "Login successful!" << std::endl;
      } else {
        std::cout << "Login failed!" << std::endl;
      }
    } else if (command == "a") {
      std::string username, password;

      std::cout << "Enter username: ";
      std::cin >> username;

      std::cout << "Enter password: ";
      std::cin >> password;

      if (0 == lm->addLogin(username, password)) {
        std::cout << "Add login successful!" << std::endl;
      } else {
        std::cout << "Add login failed!" << std::endl;
      }
    } else if (command == "d") {
      std::string username, password;
      std::cout << "Delete user." << std::endl;
      std::cout << "Enter existing username: ";
      std::cin >> username;
      std::cout << "Enter password: ";
      std::cin >> password;
      int rc = lm->delLogin(username, password);
      if (0 == rc) {
        std::cout << "Success!" << std::endl;
      } else {
        std::cout << "Failed." << std::endl;
        std::cout << "Username: " << username << ", Password: " << password
                  << " gave return code: " << rc << std::endl;
      }
    } else if (command == "c") {
      std::string username, password;
      std::cout << "Change user password." << std::endl;
      std::cout << "Enter existing username: ";
      std::cin >> username;
      std::cout << "Enter new password: ";
      std::cin >> password;
      int rc = lm->changePassword(username, password);
      if (0 == rc) {
        std::cout << "Success!" << std::endl;
      } else {
        std::cout << "Failed." << std::endl;
        std::cout << "Username: " << username << ", Password: " << password
                  << " gave return code: " << rc << std::endl;
      }
    } else if (command == "s" && !server_running) {
      std::cout << "Start server." << std::endl;
      lm->startAPI();
      server_running = true;
    } else if (command == "s" && server_running) {
      std::cout << "Stop server." << std::endl;
      lm->stopAPI();
      server_running = false;
    } else {
      std::cout << "Invalid command. Type h for help." << std::endl;
    }
  }

  if (server_running) {
    std::cout << "Stop server." << std::endl;
    lm->stopAPI();
    server_running = false;
  }

  std::cout << "Login Manager Exits." << std::endl;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    print_usage();
    return 1;
  }
  std::string db_path = "";
  Logger::LogOut log_out;
  Logger::LogLevel log_level;
  std::string logger_path = "";

  if (strcmp(argv[1], "-sp") == 0) {
    YAML::Node config = YAML::LoadFile(argv[2]);
    db_path = config["database"]["path"].as<std::string>();

    string logger_out = config["logging"]["out"].as<std::string>();
    if ("file" == logger_out || "File" == logger_out || "FILE" == logger_out) {
      log_out = Logger::LogOut::FILE;
      logger_path = config["logging"]["file"]["path"].as<std::string>();
    } else if ("stdout" == logger_out || "Stdout" == logger_out ||
               "STDOUT" == logger_out) {
      log_out = Logger::LogOut::STDOUT;
    }

    std::string logger_level = config["logging"]["level"].as<std::string>();
    if ("ERROR" == logger_level || "Error" == logger_level ||
        "error" == logger_level) {
      log_level = Logger::LogLevel::ERROR;
    } else if ("WARNING" == logger_level || "Warning" == logger_level ||
               "warning" == logger_level) {
      log_level = Logger::LogLevel::WARNING;
    } else if ("INFO" == logger_level || "Info" == logger_level ||
               "info" == logger_level) {
      log_level = Logger::LogLevel::INFO;
    }
  } else if (strcmp(argv[1], "-dp") == 0) {
    db_path = argv[2];
  } else {
    std::cout << "Invalid option: " << argv[1] << std::endl;
    print_usage();
    return 1;
  }

  std::cout << "Path: " << db_path << std::endl;

  try {
    LoginManager lm(db_path);
    if (Logger::LogOut::FILE == log_out) {
      lm.logToFile(logger_path);
    }
    if (log_level) {
      lm.setLogLevel(log_level);
    }
    event_loop(&lm);
  } catch (const std::runtime_error &e) {
    std::cerr << "Error starting Login Manager CLI: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
