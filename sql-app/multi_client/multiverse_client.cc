#include <string>
#include <iostream>

#include "multiverse_client.hh"

int main(int argc, char *argv[]){
	if(argc != 3){
		std::cout << "Usage: ./client <username> <password>" << std::endl;
		return 1;
	}
	//connect to server
	MultiverseClient client(grpc::CreateChannel("localhost:8080", grpc::InsecureChannelCredentials()));

	//login
	std::string username = std::string(argv[1]);
	std::string password = std::string(argv[2]);
	LoginReq loginInfo;
	loginInfo.set_username(username);
	loginInfo.set_password(password);
	LoginResp loggedIn;
	grpc::ClientContext ctx;
	grpc::Status r_code = client.stub->DoLogin(&ctx, loginInfo, &loggedIn);
	if(r_code.ok()){
		if(!loggedIn.success()){
			return 1;
		}
	}else{
		return 1;
	}

	//repl to process user's commands 
	std::string command;
	while(1){
		std::cout << "Enter command: ";

		std::getline(std::cin, command);
		if(std::cin.eof()){
			break;
		}else{
			if(command == "exit"){ //exit
				DisconnectReq usr;
				usr.set_username(username);
				DisconnectResp disconn;
				grpc::ClientContext ctx;
				client.stub->DisconnectUser(&ctx, usr, &disconn);
				return 0;
			}else{ //make query 
				QueryReq req;
				req.set_username(username);
				req.set_query(command);
				QueryResp resp;
				grpc::ClientContext ctx;
				r_code = client.stub->MakeQuery(&ctx, req, &resp);
				if(r_code.ok()){
					int len = resp.rows_size();
					for(int i = 0; i < len; i++){
						const Row &r = resp.rows(i);
						std::cout << r.value() << std::endl;
					}
				}else{
					//there was an error 
					std::cout << "Error" << std::endl;
				}
			}
		}
	} 
}