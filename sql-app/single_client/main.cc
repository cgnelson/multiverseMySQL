#include <stdlib.h>
#include <iostream>
#include <string>

#include "mysql_connection.h"
#include "mysql_driver.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "../sql_proxy.hh"

void check_policy(SqlProxy *prox){
  std::cout << "row policies" << std::endl;
  auto iter = prox->tableRowPolicies.begin();
  while(iter != prox->tableRowPolicies.end()){
    std::cout << iter->first << std::endl;
    for(unsigned int i = 0; i < iter->second.size(); i++){
      std::cout << iter->second[i] << " ";
    }
    std::cout << std::endl;
    iter++;
  }
  std::cout << "column policies" << std::endl;
  auto iter2 = prox->tableColPolicies.begin();
  while(iter2 != prox->tableColPolicies.end()){
    std::cout << iter2->first << std::endl;
    auto iter3 = iter2->second.begin();
    while(iter3 != iter2->second.end()){
      std::cout << iter3->first << std::endl;
      for(unsigned int i = 0; i < iter3->second.size(); i++){
        std::cout << iter3->second[i] << " ";
      }
      std::cout << std::endl;
      iter3++;
    }
    iter2++;
  }
}

void check_user_policy(SqlProxy *prox, SqlUser *user){
  std::cout << std::endl;
  std::cout << user->getUsername() << " row policies" << std::endl;
  auto iter = prox->rowPolicies[user->activeID].begin();
  while(iter != prox->rowPolicies[user->activeID].end()){
    std::cout << iter->first << std::endl;
    std::cout << iter->second << std::endl;
    iter++;
  }
  std::cout << "column policies" << std::endl;
  auto iter2 = prox->colPolicies[user->activeID].begin();
  while(iter2 != prox->colPolicies[user->activeID].end()){
    std::cout << iter2->first << std::endl;
    auto iter3 = iter2->second.begin();
    while(iter3 != iter2->second.end()){
      std::cout << iter3->first << std::endl;
      std::cout << iter3->second << std::endl;
      iter3++;
    }
    iter2++;
  }
}

void repl(SqlUser *user, SqlProxy *proxy){

  std::string command;

  while(1){
     std::cout << "Enter command: ";

     std::getline(std::cin, command);
     if(std::cin.eof()){
      break;
     }

     proxy->execute_command(user, command);
  }  
}

int main(int argc, char *argv[]){
  if(argc != 7){
    std::cout << "Proper usage: " << std::endl;
    std::cout << "./main <mysql schema> <policy file> <user username> <user password> <proxy username> < proxy password>" << std::endl;
    return 1;
  }

  SqlProxy *prox = new SqlProxy(std::string(argv[1]), std::string(argv[2]), std::string(argv[5]), std::string(argv[6]));
  //check_policy(prox);

  SqlUser *test_cli = new SqlUser(std::string(argv[3]), std::string(argv[4]), 0);

  int err = prox->connect_to_server(test_cli);
  if(err){
    delete test_cli;
    delete prox;
    return 1;
  }
  //check_user_policy(prox, test_cli);

  repl(test_cli, prox);

  delete test_cli;
  delete prox;

  return 0;
}