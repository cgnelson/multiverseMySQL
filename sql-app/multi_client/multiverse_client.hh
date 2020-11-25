#include <memory>
#include <grpcpp/grpcpp.h>
#include "multiverse.grpc.pb.h"


using grpc::Channel;
/*
using protos::MultiverseManager;
using protos::QueryReq;
using protos::Row;
using protos::QueryResp;
using protos::LoginReq;
using protos::LoginResp;
using protos::DisconnectReq;
using protos::DisconnectResp;
*/

class MultiverseClient {
public:
	std::unique_ptr<MultiverseManager::Stub> stub;

	MultiverseClient(std::shared_ptr<Channel> channel) : stub(MultiverseManager::NewStub(channel)) {}
};