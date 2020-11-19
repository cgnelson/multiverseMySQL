#include "mysql_connection.h"

#include <string>

class SqlUser {
private:
	std::string userName;
	std::string userPassword;
	int group;

public:
	int activeID;
	sql::Connection *conn;

	SqlUser(std::string username, std::string password, int g){
		userName = username;
		userPassword = password;
		group = g;
		activeID = -1;
		conn = NULL;
	}

	~SqlUser(){
		if(conn != NULL){
			delete conn;
		}
	}

	std::string getUsername(){
		return userName;
	}

	std::string getPassword(){
		return userPassword;
	}

	int getGroup(){
		return group;
	}

};