#include <string>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <string.h>

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

	//keeps track of column names for tables in the privacy policy
	std::unordered_map<std::string, std::vector<std::string>> privacySchema;

	//generic polciy over the rows of each table, use this to build user specific policies 
	std::unordered_map<std::string, std::vector<std::string>> tableRowPolicies;

	//generic polciy over the columns of each table, use this to build user specific policies 
	std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> tableColPolicies;

	//generic policy on updates on each table
	std::unordered_map<std::string, std::vector<std::string>> tableUpdatePolicies;

	//restrictions on what rows can be accessed by a user for each table
	//map user -> (map table -> policy) 
	//represented as WHERE clauses 
	std::unordered_map<int, std::unordered_map<std::string, std::string>> rowPolicies;

	//used to modify values returned for certain columns for certain users 
	//maps user to the columns which have restrictions and their corresponding restriction 
	//restrictions are represented as CASE WHEN ... THEN ... ELSE clauses 
	//map user -> (map table -> (map column -> restriction))
	std::unordered_map<int, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> colPolicies;

	//used to ammend update queries made by users so that they can only modify certain entries
	//maps user to thr table with a write restriction to the corresponding restriction 
	//Append additional WHERE restrictions 
	//map user -> (map table -> restriction)
	std::unordered_map<int, std::unordered_map<std::string, std::string>> updatePolicies;

	//maps table name to view for the user
	std::unordered_map<int, std::unordered_map<std::string, std::string>> activeViews; 

	//Queries the database to get the column names of all tables in the privacy policy 
	//Used once when initially parsing the policy
	std::vector<std::string> get_cols(std::string table_name);

	//reads from the provacy policy text file and parses and stores the resulting policy 
	//row policies get stored in tableRowPolicies
	//col policies get stored in tableColPolciies
	void parse_policy(std::string filename);

	//makes a user's specific policy based on the generic polciies stored in tableRowPolciies and tableColPolicies
	//resulting user polciies are stored in rowPolicies and colPolicies 
	std::string construct_user_policy(SqlUser *user, std::vector<std::string> temp);

	//connects a user to the MySQL server
	int connect_to_server(SqlUser *user);

	//Removes all saved data about a user. This includes their polcies, views, and connections 
	void disconnect_from_server(SqlUser *user);

	//Used in view creation. Returns all of the field a view should select applying column restrictions where applicable
	std::string get_fields(SqlUser *user, std::string table_name);

	//executes the SQL command to create a new view 
	void query_view(SqlUser *user, std::string view_name, std::string table_name);

	//creates (or gets the name of the pre-existing) user view for every protected table in a query 
	std::unordered_map<std::string, std::string> create_views(SqlUser *user, std::vector<std::string> tables);

	//replaces all mentions of protected tables in a query with the user's view of that table 
	std::string insert_view(SqlUser *user, std::string og_query, std::unordered_map<std::string, std::string> views);

	//gets all tables referenced in a given query. Stores the result in the "tables" argument 
	void get_tables(hsql::TableRef *table, std::vector<std::string>& tables, bool update);

	//adds restrictions defined by the privacy policy to the users query 
	std::string add_policy(SqlUser *user, std::string command);

	//executes the specified command and prints it to stdout 
	void execute_command(SqlUser *user, std::string command);

	//executes the specified command and returns the result in a vector. One entry per row in the result 
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
