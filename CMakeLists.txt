cmake_minimum_required(VERSION 3.10)

project(LoginManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")

# Find yaml-cpp package
find_package(yaml-cpp REQUIRED)

# Include directories
include_directories(include)
include_directories(sqlite3)
include_directories(/usr/local/include)

# Print diagnostic messages
message(STATUS "YAML_CPP_INCLUDE_DIRS: ${YAML_CPP_INCLUDE_DIRS}")
message(STATUS "YAML_CPP_LIBRARIES: ${YAML_CPP_LIBRARIES}")

# Source files
set(SOURCES
    src/login_manager.cpp
    src/database.cpp
    src/hash_password.cpp
    src/sanitizer.cpp
    sqlite3/sqlite3.c
    src/udp_server.cpp
  )
# Create a library from the source files
add_library(login_manager_lib ${SOURCES})

# Link the library with yaml-cpp
target_link_libraries(login_manager_lib /usr/local/lib/libyaml-cpp.dylib)

# Executable
add_executable(login_manager src/main.cpp)
add_executable(upd_server src/udp_server.cpp)

# Link the main executable with the library
target_link_libraries(login_manager login_manager_lib /usr/local/lib/libyaml-cpp.dylib)

# Link the main executable with the library
# target_link_libraries(login_manager login_manager_lib)

# Unit tests
enable_testing()
# Enable testing and include CTest
include(CTest)

# Test LoginManager
add_executable(test_login_manager tests/test_login_manager.cpp)
target_link_libraries(test_login_manager login_manager_lib)
add_test(NAME TestLoginManager COMMAND test_login_manager)
# Test HashPassword
add_executable(test_hash_password tests/test_hash_password.cpp)
target_link_libraries(test_hash_password login_manager_lib)
add_test(NAME TestHashPassword COMMAND test_hash_password)
# Test Sanitizer
add_executable(test_sanitizer tests/test_sanitizer.cpp)
target_link_libraries(test_sanitizer login_manager_lib)
add_test(NAME TestSanitizer COMMAND test_sanitizer)

