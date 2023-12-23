#include <absl/flags/commandlineflag.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <string>

#include "universe.grpc.pb.h"

using std::cout, std::endl, std::string, std::unique_ptr;

ABSL_FLAG(string, port, "8001", "Listening port");

class UniverseServiceImpl final : public UniverseService::Service {
  grpc::Status Ping(
      grpc::ServerContext *context,
      PingRequest const *request,
      PingResponse *response) override {
    response->set_message("Pong " + request->message());
    return grpc::Status::OK;
  }
};

int main(int argc, char **argv) {
  absl::SetProgramUsageMessage("Universe server");
  absl::ParseCommandLine(argc, argv);
  string port_string = absl::GetFlag(FLAGS_port);

  UniverseServiceImpl universe_service;

  grpc::ServerBuilder server_builder;
  int port = 0;
  server_builder.AddListeningPort(
      "localhost:" + port_string, grpc::InsecureServerCredentials(), &port);
  server_builder.RegisterService(&universe_service);

  unique_ptr<grpc::Server> server = server_builder.BuildAndStart();
  if (server == nullptr) {
    cout << "Failed to start server." << endl;
    return 1;
  }

  cout << "Universe server listening on port " << port << endl;
  server->Wait();
  return 0;
}