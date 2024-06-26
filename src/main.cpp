#include "login_manager.h"
#include <iostream>
#include <string> // Include the necessary header for std::string

int main() {
  LoginManager loginManager("database/login.db");
  std::string command;
  while (command != "q"){
    std::cout << "Enter command {l | a | q}: ";
    std::cin >> command;
    if (command == "l") {
      std::string username, password;

      std::cout << "Enter username: ";
      std::cin >> username;

      std::cout << "Enter password: ";
      std::cin >> password;

      if (loginManager.login(username, password)) {
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

      if (loginManager.addLogin(username, password)) {
          std::cout << "Add login successful!" << std::endl;
      } else {
          std::cout << "Add login failed!" << std::endl;
      }
    }
  }
  return 0;
}

