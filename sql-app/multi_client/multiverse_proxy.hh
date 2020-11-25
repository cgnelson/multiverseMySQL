#include "multiverse.grpc.pb.h"
#include "../sql_proxy.hh"

#include <unordered_map>
#include<string>

using grpc::Status;
using grpc::ServerContext;
/*
using protos::MultiverseManager;
using protos::QueryReq;
using protos::Row;
using protos::QueryResp;
using protos::LoginReq;
using protos::LoginResp;
using protos::DisconnectReq;
using protos::DisconnectResp;
using protos::Empty;
*/

class MultiverseProxy final : public MultiverseManager::Service {
private:
	SqlProxy *proxy;
	//maps username to user context
    std::unordered_map<std::string, SqlUser*> users;

public:
	MultiverseProxy(std::string schema, std::string filename, std::string username, std::string password){
		this->proxy = new SqlProxy(schema, filename, username, password);
	}
	
    Status MakeQuery(ServerContext* context, const QueryReq* request, QueryResp* response);

    Status DoLogin(ServerContext* context, const LoginReq* request, LoginResp* response);

    Status DisconnectUser(ServerContext* context, const DisconnectReq* request, DisconnectResp* response);
 
};