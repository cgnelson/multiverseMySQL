#include <string>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <string.h>
#include <pthread.h>

#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "sql_server.hh"

#include "SQLParser.h"

class SqlProxy {
public:
	//name of database that this is a proxy for
	std::string schema;

	//name of the file being used to declare the privacy policy 
	std::string configFile;

	//manages connections to the mysql server
	SqlServer *server;

	//username and password that the proxy uses
	std::string username;
	std::string password;

	std::vector<pthread_t> clients;

	//keeps track of column names for tables in the privacy policy
	std::unordered_map<std::string, std::vector<std::string>> privacySchema;

	//generic polciy over the rows of each table, use this to build user specific policies 
	std::unordered_map<std::string, std::vector<std::string>> tableRowPolicies;

	//generic polciy over the columns of each table, use this to build user specific policies 
	std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> tableColPolicies;

	//restrictions on what rows can be accessed by a user for each table
	//map user -> (map table -> policy) 
	//represented as WHERE clauses 
	std::unordered_map<int, std::unordered_map<std::string, std::string>> rowPolicies;

	//used to modify values returned for certain columns for certain users 
	//maps user to the columns which have restrictions and their corresponding restriction 
	//restrictions are represented as CASE WHEN ... THEN ... ELSE clauses 
	//map user -> (map table -> (map column -> restriction))
	std::unordered_map<int, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> colPolicies;

	//maps the materialized view for the user
	//NOT YET USED, still open for modification 
	std::unordered_map<int, std::unordered_map<std::string, std::string>> activeViews; 

	std::vector<std::string> get_cols(std::string table_name);

	void parse_policy(std::string filename);

	std::string construct_user_policy(SqlUser *user, std::vector<std::string> temp);

	int connect_to_server(SqlUser *user);

	void disconnect_from_server(SqlUser *user);

	std::string get_fields(SqlUser *user, std::string table_name);

	void query_view(SqlUser *user, std::string view_name, std::string table_name);

	std::unordered_map<std::string, std::string> create_views(SqlUser *user, std::vector<std::string> tables);

	std::string insert_view(SqlUser *user, std::string og_query, std::unordered_map<std::string, std::string> views);

	void get_tables(hsql::TableRef *table, std::vector<std::string>& tables);

	std::string add_policy(SqlUser *user, std::string command);

	void execute_command(SqlUser *user, std::string command);

	std::vector<std::string> execute_commands(SqlUser *user, std::string command);

	SqlProxy(std::string schema, std::string filename, std::string username, std::string password){
		this->schema = schema;
		this->configFile = filename;
		this->username = username;
		this->password = password;
		this->server = new SqlServer(schema);
		parse_policy(filename);
	}

	~SqlProxy(){
		delete server;
	}

};
