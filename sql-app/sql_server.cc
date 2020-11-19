#include "sql_server.hh"

int SqlServer::make_connection(SqlUser *user){
	try{
		user->conn = this->driver->connect("tcp://127.0.0.1:3306", user->getUsername(), user->getPassword());
		user->conn->setSchema(this->schema);

		user->activeID = this->nextID;
		nextID++;

		return 0;
	}catch (sql::SQLException &e) {
		std::cout << "Error setting up client: " << e.what();
		std::cout << " [MySQL error code: " << e.getErrorCode();
		std::cout << ", State: " << e.getSQLState() << "]" << std::endl;
		
		return 1;
	}
}