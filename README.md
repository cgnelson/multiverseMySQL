# MultiverseMySQL
Multiverse database support for MySQL

# Overview 
A proxy and client system for adding multiverse database to MySQL. User universes are implemented through views which sandbox a user's query to only the subset
of rows and columns they are allowed to access by the specified privacy policy. 

There are 2 run option. The first can be found in the single_client directory. Here, the main executable spawns an instance of a proxy and user instance. A client can
then send queries through the proxy via the command line. This implementation is primarily used for testing purposes as it can only support a single user at a time.

The second run option is in the multi_client directory. Here, there are both proxy and client executable. Any number of clients can connect to the proxy. Clients can
again query the database through a command line repl.

# Privacy Policies
Privacy polciies can simply be written in text files. A privacy policy has 2 keywords: TABLE, ALLOW, and REWRITE <column name>. TABLE specifies the name of the table the follwing policies apply to. ALLOW sepcifies restrictions over rows. ROW restrictions are written as WHERE clauses. REWRITE followed by a column name specifies restrcitions over that column. Column restrictions are written through CASE clauses. All policy specifiers must be written on their own line. For example, the line after TABLE is expected to the the name of a table. The line after ALLOW is epxected to be a WHERE clause (all on one line) and the line after REWRITE is expected to be a CASE clause (again all on one line). An example policy format can be found in the file policy.txt

# Run Requirements
Currently, the multiverse proxy assumes that the desired MySQL server is running locally. As such, you will need MySQL installed and up and running. 
To connect to a different server, you will have to change line 5 in sql_server.cc accordingly. 

Additionally, the proxy must have its own user on the MySQL server. This is needed so that it can Query the column names for the tables effected by 
the Privacy Policy. The proxy needs knowledge of all the column names of a table for view creation. 

# Compilation Requirements 
To compile the code yourself, the [mysqlcppconn](https://github.com/mysql/mysql-connector-cpp), [sqlparser](https://github.com/hyrise/sql-parser), and [gRPC and protobuf](https://grpc.io/docs/languages/cpp/quickstart/) 
libraries are needed.  Modifications may need to be made to multi_client/CMakeLists.txt and single_client/Makefile to reflect your install location of these libraries.  
