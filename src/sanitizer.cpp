#include <string>
#include <unordered_set>
#include <cctype>
#include <sanitizer.h>

std::unordered_set<char> Sanitizer::allowed_characters = {
    '!', '#', '$', '%', '&', '\'', '*', '+', '-', '/', '=', '?', '^', '_', '`', '{', '|', '}', '~', '.',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};

bool Sanitizer::checkEmail(const std::string& email){
  // Find the position of '@' symbol
  auto pos = email.find('@');
  
  // Ensure there is exactly one '@' and it's not at the beginning or end
  if (pos == std::string::npos || pos == 0 || pos == email.length() - 1) {
      return false;
  }
  
  // Split into local-part and domain-part
  std::string local_part = email.substr(0, pos);
  std::string domain_part = email.substr(pos + 1);

  if (!validateLocalPart(local_part)) {
      return false;
  }
  
  if (!validateDomainPart(domain_part)) {
      return false;
  }
  
  return true;
}

bool Sanitizer::validateLocalPart(const std::string& local_part) {
    if (local_part.empty()) {
        return false;
    }
    for (size_t i = 0; i < local_part.size(); ++i) {
        char c = local_part[i];
        if (c == '.') {
            // Check dot position
            if (i == 0 || i == local_part.size() - 1) {
                return false;
            }
            // Check double dots
            if (i < local_part.size() - 1) {
              if (local_part[i+1] == '.') {
                return false;
              }
            }
        } else if (allowed_characters.find(c) == allowed_characters.end()) {
            return false; // Found a character not in allowed set
        }
    } 
    return true;
}

bool Sanitizer::validateDomainPart(const std::string& domain_part) {
    // Domain part should not be empty
    if (domain_part.empty()) {
        return false;
    }
    
    std::size_t start = 0;
    std::size_t len = domain_part.length();
    std::size_t dots = 0;
    
    // Check that domain neither starts nor stop with a dot
    if (domain_part.at(start) == '.'){
	return false;
    }
    if (domain_part.at(len-1) == '.'){
	return false;
    }

    // Check each char for legit values
    char ch = domain_part.at(start);
    while (start < len-1) {
	if (std::isalnum(ch) || ch == '-'){	
	    start++;
	}
	else if (ch == '.') {
	    start++;
	    dots++;
            if (domain_part.at(start) == '.'){
	        return false;
	    }
	}
	else {
            return false;
	}
        ch = domain_part.at(start);
    }
    if (std::isalnum(ch) && dots > 0){
	return true;
    }
    else {
	return false;
    }
}
