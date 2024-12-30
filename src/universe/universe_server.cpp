#include <absl/flags/commandlineflag.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <iostream>
#include <memory>

#include "common.h"

#include "rpc.h"
#include "tasks.h"
#include "ui.h"

using std::cout, std::endl;

ABSL_FLAG(string, port, "8100", "Listening port");

int main(int argc, char **argv) {
	try {
		absl::SetProgramUsageMessage("Universe server");
		absl::ParseCommandLine(argc, argv);
		string port_string = absl::GetFlag(FLAGS_port);

		TaskQueue tasks;

		RpcServer rpc_server(argc, argv, tasks);
		rpc_server.start("localhost:" + port_string);
		cout << "Listening on port " << rpc_server.port() << endl;

		UI ui(tasks);
		ui.event_loop(&rpc_server);
	} catch (std::exception &e) {
		cout << e.what() << endl;
		return 1;
	} catch (...) {
		cout << "Unknown error" << endl;
		return 1;
	}

	return 0;
}