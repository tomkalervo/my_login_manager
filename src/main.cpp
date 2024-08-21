#include "login_manager.h"
#include <iostream>
#include <ostream>
#include <string>
#include <yaml-cpp/yaml.h>

int main(int argv, char **argc) {
  if (argv != 2) {
    std::cout << "Usage: ./login_manager path_to_settings_file" << std::endl;
    return 1;
  }

  YAML::Node config = YAML::LoadFile(argc[1]);
  std::string db_path = config["database"]["path"].as<std::string>();
  std::cout << "Path: " << db_path << std::endl;

  LoginManager loginManager(db_path);
  bool server_running = false;
  std::string command;
  while (command != "q") {
    if (server_running) {
      std::cout << "Server is running, s will stop.\n";
    } else {
      std::cout << "Server is not running, s will start.\n";
    }
    std::cout << "Enter command {l | a | s | q | help}: ";
    std::cin >> command;
    if (command == "l") {
      std::string username, password;

      std::cout << "Enter username: ";
      std::cin >> username;

      std::cout << "Enter password: ";
      std::cin >> password;

      if (0 == loginManager.login(username, password)) {
        std::cout << "Login successful!" << std::endl;
      } else {
        std::cout << "Login failed!" << std::endl;
      }
    }

    if (command == "a") {
      std::string username, password;

      std::cout << "Enter username: ";
      std::cin >> username;

      std::cout << "Enter password: ";
      std::cin >> password;

      if (0 == loginManager.addLogin(username, password)) {
        std::cout << "Add login successful!" << std::endl;
      } else {
        std::cout << "Add login failed!" << std::endl;
      }
    }

    if (command == "s" && !server_running) {
      std::cout << "Start server." << std::endl;
      loginManager.startAPI();
      server_running = true;
    } else if (command == "s" && server_running) {
      std::cout << "Stop server." << std::endl;
      loginManager.stopAPI();
      server_running = false;
    }
  }

  if (server_running) {
    std::cout << "Stop server." << std::endl;
    loginManager.stopAPI();
    server_running = false;
  }

  std::cout << "Login Manager Exits." << std::endl;
  return 0;
}
