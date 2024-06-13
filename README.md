# My SQLite3 C++ login handler
As the name states, this program manages user logins. It *will* have an API to send transactions for login, add user and delete user.
It is written in c++ & c, but the API will make it possible for any other program or framework to connect to it.

The current state is a simple prototype. Future features include adding SHA-256 hashing to passwords, local serversocket with http/json API interface, error logging and a backup store/restore.

I am working on this project in my spare time and my goal is to implement it on my home-server (linux distro). 
