#include "sql_proxy.hh"
#include "sql-parser/src/util/sqlhelper.h"
#include "sql-parser/src/sql/SelectStatement.h"

bool policy = true;
bool simplify = true;

void tokenize(char *input, std::vector<std::string>& result){
	char *word = strtok(input, " ");
	while(word != NULL){
		result.push_back(std::string(word));
		word = strtok(NULL, " ");
	}
}

std::vector<std::string> SqlProxy::get_cols(std::string table_name){
	std::vector<std::string> cols;
	std::string query = "SELECT column_name FROM information_schema.columns WHERE table_name = '" + table_name + "'";
	sql::Connection *conn = this->server->driver->connect("tcp://127.0.0.1:3306", this->username, this->password); 
	try{
		sql::Statement *stmt  = conn->createStatement(); 
		sql::ResultSet *res = stmt->executeQuery(query);
		while(res->next()){
			std::string col_name = res->getString(1);
			cols.push_back(col_name);
		}
		if(stmt != NULL){
			delete stmt;
		}
		if(res != NULL){
			delete res;
		}
	}catch(sql::SQLException &e){
		std::cout << "Error reading columns: " << e.what();
    	std::cout << " (MySQL error code: " << e.getErrorCode();
    	std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
	if(conn != NULL){
		delete conn;
	}
	return cols;
}

void SqlProxy::parse_policy(std::string filename){
	std::ifstream cfile(filename);

	std::string table_name;

	std::string line;
	std::vector<std::string> parsed;
	while(std::getline(cfile, line)){
		tokenize((char *)line.c_str(), parsed);
		if(parsed[0] == "TABLE"){
			parsed.clear();
			std::getline(cfile, line);

			tokenize((char *)line.c_str(), parsed);

			table_name = parsed[0];
			std::vector<std::string> cols = get_cols(table_name);
			privacySchema[table_name] = cols; 
		}else if(parsed[0] == "ALLOW"){
			parsed.clear();
			std::getline(cfile, line);

			tokenize((char *)line.c_str(), parsed);

			tableRowPolicies[table_name] = parsed;
		}else if(parsed[0] == "REWRITE"){
			assert(parsed.size() > 1);
			std::string col_name = parsed[1];

			parsed.clear();
			std::getline(cfile, line);

			tokenize((char *)line.c_str(), parsed);

			tableColPolicies[table_name][col_name] = parsed;
		}else if(parsed[0] == "MOD"){
			parsed.clear();
			std::getline(cfile, line);

			tokenize((char *)line.c_str(), parsed);

			tableUpdatePolicies[table_name] = parsed;
		}
		parsed.clear();
	}
}

std::string SqlProxy::construct_user_policy(SqlUser *user, std::vector<std::string> temp){
	std::string result = "";
	for(unsigned int i = 0; i < temp.size(); i++){
		std::size_t found = temp[i].find("self");
		if(found != std::string::npos){
			if(temp[i] == "self"){
				result += "'" + user->getUsername() + "' ";
			}else{
				if(found != 0){
					result += temp[i].substr(0, found);
				}
				result += "'" + user->getUsername() + "'";
				result += temp[i].substr(found+4, temp[i].length()-(found+4)) + " ";
			}
		}else{
			result += temp[i] + " ";
		}
	}
	return result;
}

/**
 * Pre-computes some subqueries in a user's privacy policy 
 */
std::string simplify_policy(std::string init_policy, SqlUser *user){
	// Several assumptions are made when doing this optimization:
	// 1) The policy writer has wrapped the argument to IN in parenthesis 
	// 2) There is always a space separating IN and its argument 
	// 3) IN statements in raw policies are always followed by a query and not hard coded values
	// 4) The query is only selecting one column 
	std::string policy = "";
	size_t start = 0;
	size_t pos = init_policy.find("IN");
	while(pos != std::string::npos){
		//get the query substring, assume it's wrapped in parens
		// +1 -> "N", +2 -> " ", +3 -> "(" 
		size_t len = pos - start;
		std::string sub = init_policy.substr(start, len);
		policy += sub + "IN (";
		//construct query 
		std::string query = "";
		int parens = 1;
		bool done = false;
		size_t i = pos + 4;
		while(!done){
			//keep track of parenthesis to see if we've reached end of nested query
			if(init_policy[i] == '('){
				parens++;
			}
			if(init_policy[i] == ')'){
				parens--;
			}
			//check if we've reachd end
			if(parens == 0){
				done = true; //really isn't necessary 
				break;
			}
			//add character to query 
			query += init_policy[i];
			i++;
		}
		//make query 
		std::string results = "";
		sql::ResultSet *res;
		sql::Statement *stmt;
		try{
        	stmt = user->conn->createStatement();
        	//std::cout << query << std::endl;
        	res = stmt->executeQuery(query); 
        	while(res->next()){
        		if(results != ""){
        			results += ", ";
        		}
        		results += res->getString(1);
        	}
    	}catch (sql::SQLException &e) {
    		std::cout << "Query Error: " << e.what();
    		std::cout << " (MySQL error code: " << e.getErrorCode();
    		std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    	}
    	if(res != NULL){
    		delete res;
    	}
    	if(stmt != NULL){
    		delete stmt;
    	}
    	//std::cout << results << std::endl;
    	//add results to policy 
    	policy += results + ")";
    	//update starting position 
    	start = i + 1;
    	//look again
    	pos = init_policy.find("IN", start);
	}
	//check if there us more to add
	if(start != init_policy.length()){
		policy += init_policy.substr(start);
	}
	//std::cout << "Resulting policy: " << std::endl;
	//std::cout << policy << std::endl;
	return policy;
}

int SqlProxy::connect_to_server(SqlUser *user){
	int ret = this->server->make_connection(user);

	if(ret){
		//error
		return ret;
	}else{
		//apply privacy policy for user 
		//First, construct row policy for each table for user
		auto r_it = tableRowPolicies.begin();
		while(r_it != tableRowPolicies.end()){
			std::string policy = construct_user_policy(user, r_it->second);
			if(simplify){
				std::string policy_simple = simplify_policy(policy, user);
				rowPolicies[user->activeID][r_it->first] = policy_simple;
			}else{
				rowPolicies[user->activeID][r_it->first] = policy;
			}
			r_it++;
		}
		//Then, construct col policy for each effected col of each table for user
		auto table_it = tableColPolicies.begin();
		while(table_it != tableColPolicies.end()){
			std::string table = table_it->first;
			auto c_it = table_it->second.begin();
			while(c_it != table_it->second.end()){
				std::string policy = construct_user_policy(user, c_it->second);
				if(simplify){
					std::string policy_simple = simplify_policy(policy, user);
					colPolicies[user->activeID][table][c_it->first] = policy_simple;
				}else{
					colPolicies[user->activeID][table][c_it->first] = policy;
				}
				c_it++;
			}
			table_it++;
		}
		//Finally, construct update policy for each table for the user 
		auto u_it = tableUpdatePolicies.begin();
		while(u_it != tableUpdatePolicies.end()){
			std::string policy = construct_user_policy(user, u_it->second);
			if(simplify){
				std::string policy_simple = simplify_policy(policy, user);
				updatePolicies[user->activeID][u_it->first] = policy_simple;
			}else{
				updatePolicies[user->activeID][u_it->first] = policy;				
			}
			u_it++;
		}
	}
	return ret;
}

void SqlProxy::disconnect_from_server(SqlUser *user){
	assert(user != NULL);

	//erase user entries from data structures:
	rowPolicies.erase(user->activeID);
	colPolicies.erase(user->activeID);
	activeViews.erase(user->activeID);

    //now the user can be deleted 
    delete user; 
}

std::string SqlProxy::get_fields(SqlUser *user, std::string table_name){
	std::string fields = "";
	for(unsigned int i = 0; i < privacySchema[table_name].size(); i++){
		std::string col = privacySchema[table_name][i];
		auto iter = colPolicies[user->activeID][table_name].find(col);
		if(iter != colPolicies[user->activeID][table_name].end()){
			std::string restriction = iter->second;
			fields += restriction;
			fields += "AS " + col;
			if(i != privacySchema[table_name].size()-1){
				fields += ", ";
			}
		}else{
			fields += col;
			if(i != privacySchema[table_name].size()-1){
				fields += ", ";
			}
		}
	}
	return fields;
}

void SqlProxy::query_view(SqlUser *user, std::string view_name, std::string table_name){
	std::string query = "CREATE OR REPLACE VIEW " + view_name + " AS SELECT  ";
	query += get_fields(user, table_name);
	query += " FROM " + table_name + " " + rowPolicies[user->activeID][table_name];
	//std::cout << "View query " << query << std::endl;
	sql::Statement *stmt = NULL;
	
	try{
		stmt = user->conn->createStatement();
        stmt->execute(query); 
	}catch(sql::SQLException &e){
    	std::cout << "Error creating view: " << e.what();
    	std::cout << " (MySQL error code: " << e.getErrorCode();
    	std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }

    if(stmt != NULL){
    	delete stmt;
    }
}

std::unordered_map<std::string, std::string> SqlProxy::create_views(SqlUser *user, std::vector<std::string> tables){
	//create a view based on the user's policy and return the name of that view 
	std::unordered_map<std::string, std::string> views;
	for(unsigned int i = 0; i < tables.size(); i++){
		auto found = views.find(tables[i]);
		if(found == views.end()){
			std::string view_name = tables[i]+user->getUsername();
			//check to see if view already exists:
			auto found = activeViews[user->activeID].find(tables[i]);
			if(found == activeViews[user->activeID].end()){
				std::cout << "making view" << std::endl;
				query_view(user, view_name, tables[i]);
				activeViews[user->activeID][tables[i]] = view_name;
			}
			views[tables[i]] = view_name;
		}
	}
	return views;
}

void SqlProxy::get_tables(hsql::TableRef *table, std::vector<std::string>& tables, bool update){
	if(update){
		assert(table != NULL);
		std::string table_name = std::string(table->name);
		auto found = tableUpdatePolicies.find(table_name);
		if(found != tableUpdatePolicies.end()){
			//there is a policy for this table 
			tables.push_back(table_name);
		}
	}else{
		if(table->type == hsql::kTableName){
			std::string table_name = std::string(table->name);
			//check if table exists in privacy policy 
			auto found = tableRowPolicies.find(table_name);
			if(found != tableRowPolicies.end()){
				//there is a policy for this table 
				tables.push_back(table_name);
			}else{
				auto found2 = tableColPolicies.find(table_name);
				if(found2 != tableColPolicies.end()){
					tables.push_back(table_name);
				}
			}
		}else if(table->type == hsql::kTableJoin){
			get_tables(table->join->left, tables, false);
			get_tables(table->join->right, tables, false);
		}else if(table->type == hsql::kTableCrossProduct){
			for(hsql::TableRef* tbl : *table->list){
				get_tables(tbl, tables, false);
			}
		}else if(table->type == hsql::kTableSelect){
			get_tables(table->select->fromTable, tables, false);
		}
	}
}

std::string SqlProxy::insert_view(SqlUser *user, std::string og_query, std::unordered_map<std::string, std::string> views){
	std::string new_query = "";
	char *token = strtok((char *)og_query.c_str(), " ");
	while(token != NULL){
		std::string word = std::string(token);
		auto found = views.find(word);
		if(found != views.end()){
			new_query += views[word] + " ";
		}else{
			new_query += word + " ";
		}
		token = strtok(NULL, " ");
	} 
	return new_query;
}

std::string SqlProxy::add_policy(SqlUser *user, std::string command){
	//frist, parse the query
	hsql::SQLParserResult result;
    hsql::SQLParser::parse(command, &result);
    if(result.getStatements().size() == 0){
    	std::cout << "Could not parse command" << std::endl;
    	return "panic";
    }

    const hsql::SQLStatement *stmt = result.getStatement(0);
    //printStatementInfo(stmt);
    //std::cout << "table: " << ((const hsql::SelectStatement*)stmt)->fromTable->name << std::endl;
    if(stmt->type() == hsql::kStmtSelect){
    	const hsql::SelectStatement *select_stmt = (const hsql::SelectStatement*) stmt;
    	//get all tables from this query 
    	std::vector<std::string> tables;
    	get_tables(select_stmt->fromTable, tables, false);
    	std::unordered_map<std::string, std::string> views = create_views(user, tables);
    	std::string command2 = insert_view(user, command, views);
    	//std::cout << "Returning command " << command2 << std::endl;
    	return command2;
    }else if(stmt->type() == hsql::kStmtUpdate){
    	const hsql::UpdateStatement *update_stmt = (const hsql::UpdateStatement*) stmt;
    	//get tables being updated. There will only be 1
    	std::vector<std::string> tables;
    	get_tables(update_stmt->table, tables, true);
    	if(command.find("WHERE") == std::string::npos){
    		//if no where where clause, add full policy
    		command += " " + this->updatePolicies[user->activeID][tables[0]]; 
    	}else{
    		//don't need to add "WHERE" just the restriction 
    		//policy must start with "WHERE ", so add 6 to start 
    		std::string full = this->updatePolicies[user->activeID][tables[0]];
    		command += " AND " + full.substr(6);
    	}
    	//std::cout << command << std::endl;
    	return command;
    }else{
    	return command;
    }
}

void SqlProxy::execute_command(SqlUser *user, std::string command){
	sql::Statement *stmt = NULL;
  	sql::ResultSet *res = NULL;

  	try{
  		std::string pcommand = add_policy(user, command);
  		if(pcommand == "panic"){
  			return;
  		}

        stmt = user->conn->createStatement();
        //std::cout << "new command: " << pcommand << std::endl;
        stmt->execute(pcommand); 

        do {
        	res = stmt->getResultSet();

        	if(res != NULL){
        		int cols = res->getMetaData()->getColumnCount();
        		while(res->next()){
        			for(int i = 1; i <= cols; i++){
        				std::cout << res->getString(i);

        				if(i != cols){
        					std::cout << ", ";
        				}else{
        					std::cout << std::endl;
        				}
        			}
        		}
        	}
        }while(stmt->getMoreResults());

    }catch (sql::SQLException &e) {
    	std::cout << "Query Error: " << e.what();
    	std::cout << " (MySQL error code: " << e.getErrorCode();
    	std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }

    if(stmt != NULL){
    	delete stmt;
    }
    if(res != NULL){
    	delete res;
    }
}

std::vector<std::string> SqlProxy::execute_commands(SqlUser *user, std::string command){
	std::vector<std::string> result;
	sql::Statement *stmt = NULL;
  	sql::ResultSet *res = NULL;

  	try{
  		std::string pcommand;
  		if(policy){
  			pcommand = add_policy(user, command);
  			if(pcommand == "panic"){
  				return result;
  			}
  		}else{
  			pcommand = command;
  		}

        stmt = user->conn->createStatement();
        //std::cout << "new command: " << pcommand << std::endl;
        stmt->execute(pcommand); 

        do {
        	res = stmt->getResultSet();

        	if(res != NULL){
        		int cols = res->getMetaData()->getColumnCount();
        		while(res->next()){
        			std::string row = "";
        			for(int i = 1; i <= cols; i++){
        				row += res->getString(i);
        				if(i != cols){
        					row += ", ";
        				}else{
        					result.push_back(row);
        				}
        			}
        		}
        	}
        }while(stmt->getMoreResults());

    }catch (sql::SQLException &e) {
    	std::cout << "Query Error: " << e.what();
    	std::cout << " (MySQL error code: " << e.getErrorCode();
    	std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }

    if(stmt != NULL){
    	delete stmt;
    }
    if(res != NULL){
    	delete res;
    }

    return result;
}
