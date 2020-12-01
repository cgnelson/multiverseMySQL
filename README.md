# MultiverseMySQL
Multiverse database support for MySQL

# Overview 
A proxy and client system for adding multiverse database to MySQL. User universes are implemented through views which sandbox a user's query to only the subset of rows and columns they are allowed to access by the specified privacy policy. 

There are 2 run options. The first can be found in the single_client directory. Here, the main executable spawns an instance of a proxy and user instance. A client can then send queries through the proxy via the command line. This implementation is primarily used for testing purposes as it can only support a single user at a time. 

The single client implementation can be run with ./main {desired mysql schema} {policy file} {client's username} {client's password} {proxy's username} {proxy's password} 

The second run option is in the multi_client directory. Here, there are both proxy and client executables. Any number of clients can connect to the proxy. Clients can again query the database through a command line repl. Currently, the proxy and client are coded to run on the same machine (as can be seen on line 64 of multiverse_proxy.cc and and 14 of multiverse_client.cc). These lines can be changed if required.

The proxy can be run with: ./proxy {desired mysql schema} {policy file} {proxy's username} {proxy's password}

The client can be run with: ./client {client's username} {client's password} {sql command file (optinal)}

Additionally, in the generator folder, a file generator.cc can be used to create a small testing database (modeled after Piazza). In order to use this generator, you must have already set up a piazza schema in your MySQL database and at least one user authorized to access it. It must at least have the table posts(post_id int, username varchar, class int, post text, visability int, anon int, time_posted int). The generator creates 100 posts to a piazza database with a piazza table. There are 2 possible classes used (1 and 2), and 5 possible users (prof, prof2, bob, alice, mike). To log in as one of these users, you will have to create users on your MySQL database with their name as the username. 

# Privacy Policies
Privacy polciies can simply be written in text files. A privacy policy has 4 keywords: TABLE, ALLOW, REWRITE "column name", and MOD. TABLE specifies the name of the table the follwing policies apply to. ALLOW sepcifies restrictions over rows. Row restrictions are written as WHERE clauses. REWRITE followed by a column name specifies restrcitions over that column. Column restrictions are written through CASE clauses. MOD specifies restrictions on updates. Currently, updates are only restricted at row granularity. All policy specifiers must be written on their own line. For example, the line after TABLE is expected to the the name of a table, the line after ALLOW is epxected to be a WHERE clause (all on one line), and the line after REWRITE is expected to be a CASE clause (again all on one line). An example policy format can be found in the file policy.txt.

If you would like to test out the example privacy policy, you must have your database set up with a piazza schema with at least 2 tables: posts(post_id int, username varchar, class int, post text, visability int, anon int, time_posted int) and people(username varchar, class int, position varchar).

# Run Requirements
Currently, the multiverse proxy assumes that the desired MySQL server is running locally. As such, you will need [MySQL installed](https://www.digitalocean.com/community/tutorials/how-to-install-mysql-on-ubuntu-18-04) and up and running. To connect to a different server, you will have to change line 5 in sql_server.cc accordingly. 

Additionally, the proxy must have its own user on the MySQL server. This is needed so that it can Query the column names for the tables effected by the Privacy Policy. The proxy needs knowledge of all the column names of a table for view creation. 

Before the proxy can be used, you must have already created 1 or more MySQL schemas and at least 1 user with access permission to this schema. The easiest way to do this is through the MySQL commandline client (logged in as root). Schemas can be created with the command CREATE DATABASE name. Users can be created with the command CREATE USER 'name'@'localhost' IDENTIFIED BY 'password'. Users can be granted permissions with the command GRANT {permission} ON {databse}.{table} TO 'name'@'localhost'.

# Compilation Requirements 
To compile the code yourself, the [mysqlcppconn](https://github.com/mysql/mysql-connector-cpp), [sqlparser](https://github.com/hyrise/sql-parser), and [gRPC and protobuf](https://grpc.io/docs/languages/cpp/quickstart/) 
libraries are needed.  Modifications may need to be made to multi_client/CMakeLists.txt and single_client/Makefile to reflect your install location of these libraries.  

# Bugs
There are some bugs in this implementation that carry over from the hyrise sqlparser. For example, certain instances of invalid SQL or using unsuprted features of the language cause the parser to segfault. Additionally, the parser does not properly remove parenthesis when finding table names in queries. As such, a query such as (SELECT * FROM table) will not apply the privacy policy on "table" because the parser does not recognize it as being in the query. For the most reliable performance, please space separate parenthesis, such as ( SELECT * FROM table )
