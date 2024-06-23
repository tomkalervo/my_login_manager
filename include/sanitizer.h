#ifndef SANITIZER_H
#define SANITIZER_H

#include <string>
#include <unordered_set>
#include <sanitizer.h>

class Sanitizer {
public:
  static bool checkEmail(const std::string& email);
private:
  static std::unordered_set<char> allowed_characters;
  static bool validateLocalPart(const std::string& local_part);
  static bool validateDomainPart(const std::string& domain_part);
};

#endif // SANITIZER_H

