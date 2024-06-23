#include <iostream>
#include <cassert>
#include "sanitizer.h"

void testCheckEmail() {
    // Valid email addresses
    assert(Sanitizer::checkEmail("test@example.com") == true);
    assert(Sanitizer::checkEmail("user.name+tag+sorting@example.com") == true);
    assert(Sanitizer::checkEmail("user_name@example.co.uk") == true);

    // Invalid email addresses
    assert(Sanitizer::checkEmail("plainaddress") == false);
    assert(Sanitizer::checkEmail("@missingusername.com") == false);
    assert(Sanitizer::checkEmail("username@.com") == false);
    assert(Sanitizer::checkEmail("username@.com.") == false);
    assert(Sanitizer::checkEmail("username@.com..com") == false);

    std::cout << "All email tests passed!" << std::endl;
}

int main() {
    testCheckEmail();

    std::cout << "All tests passed!" << std::endl;

    return 0;
}

