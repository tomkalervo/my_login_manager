#include "login_manager.h"
#include <iostream>
#include <ostream>
#include <string>
#include <cstring>
#include <yaml-cpp/yaml.h>
#include <stdexcept>

void print_usage(){
  std::cout << "./login_manager [-sp path_to_settings] | [-dp path_to_database] \n\n";

  std::cout << "Options:\n";
  std::cout << "  -sp  Path to the settings file.\n";
  std::cout << "  -dp  Path to the database file.\n\n";

  std::cout << "Note: You must provide either the path to a settings file with `-sp`";
  std::cout << " or a path to a database file with `-dp`, but not both." << std::endl;

}
void event_loop(LoginManager* lm){
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

      if (0 == lm->login(username, password)) {
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

      if (0 == lm->addLogin(username, password)) {
        std::cout << "Add login successful!" << std::endl;
      } else {
        std::cout << "Add login failed!" << std::endl;
      }
    }

    if (command == "s" && !server_running) {
      std::cout << "Start server." << std::endl;
      lm->startAPI();
      server_running = true;
    } else if (command == "s" && server_running) {
      std::cout << "Stop server." << std::endl;
      lm->stopAPI();
      server_running = false;
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

  if (strcmp(argv[1],"-sp") == 0){
    YAML::Node config = YAML::LoadFile(argv[2]);
    db_path = config["database"]["path"].as<std::string>();
  } else if (strcmp(argv[1], "-dp") == 0){
    db_path = argv[2];
  } else {
    std::cout << "Invalid option: " << argv[1] << std::endl;
    print_usage();
    return 1;
  }
  
  std::cout << "Path: " << db_path << std::endl;

  try {
    LoginManager lm(db_path);
    event_loop(&lm);
  } catch (const std::runtime_error &e) {
    std::cerr << "Error starting Login Manager CLI: " << e.what() <<  std::endl;
    return 1;
  }
    
  return 0;
}
