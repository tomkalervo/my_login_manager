# My SQLite3 C++ login handler
As the name states, this program manages user logins. It is written in c++ & c, but the API enables any other program or framework to communicate using sockets. The aim is to only have the necessary functions for handling a database with logins. Enabling a high security by reducing the potential interactions with the database along with secure handling of passwords and SQL interactions.

The current state is still an early prototype. The Login Manager can add users, delete users and verify a userpassword. It uses a custom built hashing-algorithm (SHA-256) for passwords, it also uses salt logic for extra security. There is also a sanitizer for validating e-mail. It implements a local serversocket with my custom API interface.

I am working on this project in my spare time and my goal is to implement it on my home-server (linux distro). Feel free to reach out if interested in this project!
