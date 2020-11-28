#include <string>
#include <iostream>
#include <fstream>
#include <chrono>

#include "multiverse_client.hh"

int main(int argc, char *argv[]){
	if(argc != 3 && argc != 4){
		std::cout << "Usage: ./client <username> <password> [command file]" << std::endl;
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

	if(argc == 4){
		//start timing
		auto start = std::chrono::high_resolution_clock::now();
		//read commands from file
		std::string file_name = std::string(argv[3]);
		std::ifstream command_file;
		command_file.open(file_name, std::ifstream::in);
		std::string query;
		while(std::getline(command_file, query)){
			QueryReq req;
			req.set_username(username);
			req.set_query(query);
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
		//stop timing 
		auto stop = std::chrono::high_resolution_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
		std::cout << "////////////////////////////" << std::endl;
		std::cout << "Time: " << diff.count() << " seconds" << std::endl;
		//disconnect user
		DisconnectReq usr;
		usr.set_username(username);
		DisconnectResp disconn;
		grpc::ClientContext ctx;
		client.stub->DisconnectUser(&ctx, usr, &disconn);
		return 0;
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