#include <iostream>
#include <string>
#include "hash_password.h"
using std::string;

void testHashPassword() {
  const string empty_hash = HashPassword::usingSHA256("");
  const string empty_hash_target = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
  std::cout << "01 empty_hash string: " << empty_hash << "\n"; 
  std::cout << "01 empty_hash_target string: " << empty_hash_target << "\n"; 
  if (empty_hash == empty_hash_target) {
      std::cout << "01 Hash empty string test passed." << std::endl;
  } else {
      std::cout << "01 Hash empty string test failed." << std::endl;
  }
}

int main() {
    testHashPassword();
    return 0;
}

