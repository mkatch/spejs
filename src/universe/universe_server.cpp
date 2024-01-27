#include <absl/flags/commandlineflag.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <iostream>
#include <memory>
#include <string>

#include "rpc.h"
#include "ui.h"

using std::cout, std::endl, std::string;

ABSL_FLAG(string, port, "8100", "Listening port");

int main(int argc, char **argv) {
	absl::SetProgramUsageMessage("Universe server");
	absl::ParseCommandLine(argc, argv);
	string port_string = absl::GetFlag(FLAGS_port);

	RpcServer rpc_server;
	rpc_server.start("localhost:" + port_string);
	cout << "Listening on port " << rpc_server.port() << endl;

	UI ui;
	ui.event_loop(&rpc_server);

	return 0;
}