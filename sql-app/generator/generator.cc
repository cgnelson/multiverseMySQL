//post_id -> integer incremented with each iteration 
//username -> random interger from 0 to 4 index into array ["prof", "prof2", "bob", "alice", "mike"]
//class -> random into 1 or 2 (mod 2 + 1)
//post -> 
//visibility -> random int 0 or 1
//anon -> if visibility = 0, then 0 else random int 0 or 1
//time -> RND (DATETIME())
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>

#include "mysql_connection.h"
#include "mysql_driver.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

int count = 0; 
std::string usernames[5] = {"prof", "prof2", "bob", "alice", "mike"};
std::string letters = "ABCDEFGH IJKLMNOP QRSTUVWXYZ abcdefg hijklmno pqrstuvwxyz";
int mintime = 1542596665;
int maxtime = 1605755065;

std::string make_query(){
	//int post_id = count;
	//count++;
	int user_index = rand() % 5;
	std::string user = usernames[user_index];
	int class_name = (rand() % 2) + 1;
	int vis = rand() % 2;
	int time = (rand() % (maxtime-mintime)) + mintime;
	int anon;
	if(vis == 0){
		anon = 0;
	}else{
		anon = rand() % 2;
	}
	std::string post;
	for(int i = 0; i < 10; i++){
		post += letters[rand() % 57];
	}
	std::string query = "INSERT INTO posts(username, class, post, visability, anon, time_posted) VALUES(";
	//query += std::to_string(post_id) + ", ";
	query += "'" + user + "', ";
	query += std::to_string(class_name) + ", ";
	query += "'" + post + "', ";
	query += std::to_string(vis) + ", ";
	query += std::to_string(anon) + ", ";
	query += std::to_string(time) + ")";
	std::cout << query << std::endl;
	return query;
}

int generate_entry(sql::Connection *conn){
	try{
		sql::Statement *stmt = NULL;
		stmt = conn->createStatement();
		std::string query = make_query();
		stmt->execute(query); 

		if(stmt != NULL){
			delete stmt;
		}

		return 0;
	}catch (sql::SQLException &e) {
		std::cout << "Error: " << e.what();
		std::cout << " [MySQL error code: " << e.getErrorCode();
		std::cout << ", State: " << e.getSQLState() << "]" << std::endl;
		
		return 1;
	}
}

int main(int argc, char *argv[]){
	srand(time(NULL));
	sql::mysql::MySQL_Driver *driver = NULL;
	sql::Connection *conn = NULL;

	try{
		driver = sql::mysql::get_driver_instance();
		conn = driver->connect("tcp://127.0.0.1:3306", "demo", "casey123");
		conn->setSchema("piazza");
	}catch (sql::SQLException &e) {
		std::cout << "Error: " << e.what();
		std::cout << " [MySQL error code: " << e.getErrorCode();
		std::cout << ", State: " << e.getSQLState() << "]" << std::endl;
		
		return 1;
	}

	for(int i = 0; i < 100; i++){
		int ret = generate_entry(conn);
		if(ret){
			return ret;
		}
	}

	if(driver != NULL){
		delete driver;
	}
	if(conn != NULL){
		delete conn;
	}

	return 0;
}
