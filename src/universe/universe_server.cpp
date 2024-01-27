#include <absl/flags/commandlineflag.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <iostream>
#include <memory>
#include <string>

#include "rpc.h"

using std::cout, std::endl, std::string;

ABSL_FLAG(string, port, "8100", "Listening port");

int main(int argc, char **argv) {
	absl::SetProgramUsageMessage("Universe server");
	absl::ParseCommandLine(argc, argv);
	string port_string = absl::GetFlag(FLAGS_port);

	std::unique_ptr<RpcServer> rpc_server = RpcServer::build_and_start("localhost:" + port_string);
	cout << "Listening on port " << rpc_server->port() << endl;
	rpc_server->wait();

	return 0;
}