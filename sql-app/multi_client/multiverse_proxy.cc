#include <vector>
#include <iostream>
#include <memory>

#include "multiverse_proxy.hh"
#include <grpcpp/grpcpp.h>

bool testing = false;


::grpc::Status MultiverseProxy::MakeQuery(::grpc::ServerContext *context, const QueryReq *request, QueryResp *response){
	SqlUser *user = this->users[request->username()];
	std::string command = request->query();

	std::vector<std::string> resp = this->proxy->execute_commands(user, command);
	int max = 0;
	if(testing){
		if(resp.size() < 1){
			max = resp.size();
		}else{
			max = 1;
		}
	}else{
		max = resp.size();
	}
	for(int i = 0; i < max; i++){
		auto to_add = response->add_rows();
		to_add->set_value(resp[i]);
	}

	return ::grpc::Status::OK;
}

::grpc::Status MultiverseProxy::DoLogin(::grpc::ServerContext *context, const LoginReq *request, LoginResp *response){
	std::string username = request->username();
	std::string password = request->password();
	SqlUser *cli = new SqlUser(username, password, 0);

	int err = this->proxy->connect_to_server(cli);
	if(err){
		response->set_success(false);
	}else{
		this->users[username] = cli;
		response->set_success(true);
	}

	return ::grpc::Status::OK;
}

::grpc::Status MultiverseProxy::DisconnectUser(::grpc::ServerContext *context, const DisconnectReq *request, DisconnectResp *response){
	std::string username = request->username();
	SqlUser *user = this->users[username];
	this->proxy->disconnect_from_server(user);

	return ::grpc::Status::OK;
}

int main(int argc, char *argv[]){
	if(argc != 5){
		std::cout << "Usage: ./proxy <schema> <policy> <proxy username> <proxy password>" << std::endl;
		return 1;
	}

	std::string addr("0.0.0.0:8080");

	std::string schema = std::string(argv[1]);
	std::string policy_file = std::string(argv[2]);
	std::string username = std::string(argv[3]);
	std::string password = std::string(argv[4]);	
	MultiverseProxy proxy(schema, policy_file, username, password);

	grpc::ServerBuilder sb;
    sb.AddListeningPort(addr, grpc::InsecureServerCredentials());


    sb.RegisterService(&proxy);


    std::unique_ptr<grpc::Server> server(sb.BuildAndStart());
    
    server->Wait();
    return 0;
}