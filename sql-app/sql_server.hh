#include "mysql_connection.h"
#include "mysql_driver.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>

#include <string>
#include <stdlib.h>
#include <iostream>

#include "sql_user.hh"


class SqlServer {
public:
	int nextID;
	std::string schema;
	sql::mysql::MySQL_Driver *driver;

	SqlServer(std::string s){
		nextID = 0;
		schema = s;
		try{
			driver = sql::mysql::get_driver_instance();
		}catch (sql::SQLException &e) {
			std::cout << "Error in server construction: " << e.what();
			std::cout << " [MySQL error code: " << e.getErrorCode();
			std::cout << ", State: " << e.getSQLState() << "]" << std::endl;
			driver = NULL;
		}
	}

	~SqlServer(){
		if(driver != NULL){
			delete driver;
		}
	}

	int make_connection(SqlUser *user);
};