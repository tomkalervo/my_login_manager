# My SQLite3 C++ Login Manager
### A fast, lightweight and secure application for storing user credentials 
As the name states, this program manages user logins. It is written in c++ & c, but has a built in API that enables any other program or framework to communicate using sockets. The aim is to only have the necessary functions for handling a database with logins. Enabling a high security by reducing the potential interactions with the database along with secure handling of passwords and SQL interactions.

The current state is still an early prototype. The Login Manager can add users, delete users and verify a userpassword. It uses a custom built hashing-algorithm (SHA-256) for passwords, it also uses salt logic for extra security. There is also a sanitizer for validating e-mail. It implements a local serversocket with my custom API interface.

I am working on this project in my spare time and my goal is to implement it on my home-server (linux distro). Feel free to reach out if interested in this project!

## How-To
You can either modify or link the source code into you own c++ application. Or, build the application using cmake and run the CLI:
```console
> cmake --build build
```
Make sure you have configured the settings for the database (as of now only the path to db is used):
```console
cat config/settings.yaml
database:
  path: PATH_TO_DATABASE/login.db
  user: db_user
  password: db_password
  name: login_database
```
When build is complete, run the application:
```console
❯ ./build/login_manager -sp config/settings.yaml
```
That is all, now you can do manual add, login and delete operations as well as to start the API server.
