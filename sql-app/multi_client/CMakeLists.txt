cmake_minimum_required(VERSION 2.8)
project(multiverse_sql)

set(CMAKE_CXX_STANDARD 14)

# Find Protobuf installation
# Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC PATHS /usr/local/grpc NO_DEFAULT_PATH CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Proto files
get_filename_component(multiverse_proto "multiverse.proto" ABSOLUTE)
get_filename_component(multiverse_proto_path "${multiverse_proto}" PATH)

# Generated sources
set(multiverse_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/multiverse.pb.cc")
set(multiverse_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/multiverse.pb.h")
set(multiverse_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/multiverse.grpc.pb.cc")
set(multiverse_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/multiverse.grpc.pb.h")

add_custom_command(
        OUTPUT "${multiverse_proto_srcs}" "${multiverse_proto_hdrs}" "${multiverse_grpc_srcs}" "${multiverse_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${multiverse_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${multiverse_proto}"
        DEPENDS "${multiverse_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
# Include mysqlcppconn
set(MYSQL_CONN_CPP_DIR_PATH /usr/local/mysql/connector-c++-)
include_directories(${MYSQL_CONN_CPP_DIR_PATH}/include)
link_directories(${MYSQL_CONN_CPP_DIR_PATH}/lib64)
# include the parser
include_directories(/usr/local/include/hsql)

add_executable(client ${multiverse_proto_srcs} ${multiverse_grpc_srcs} multiverse_client.cc)
target_link_libraries(client ${_PROTOBUF_LIBPROTOBUF} ${_GRPC_GRPCPP_UNSECURE})

add_executable(proxy ${multiverse_proto_srcs} ${multiverse_grpc_srcs} multiverse_proxy.cc ../sql_user.hh ../sql_proxy.cc ../sql_proxy.hh ../sql_server.cc ../sql_server.hh)
target_link_libraries(proxy ${_PROTOBUF_LIBPROTOBUF} ${_GRPC_GRPCPP_UNSECURE} mysqlcppconn sqlparser)