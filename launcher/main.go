package main

import (
	"flag"
	"fmt"
	"maps"
	"os"
	"slices"
	"strings"
)

func main() {
	initLog()
	flag.Parse()

	_, err := os.Stat("go.work")
	if err != nil {
		log.Errorf("This program must be run from the root of the repository.")
		os.Exit(1)
	}

	universeGrpcPort := 6200
	frontendGrpcPort := 6100

	jobs := []*job{
		{
			name:  "universe",
			color: 45,
			path:  "build/Release/universe_server",
			args: []string{
				fmt.Sprintf("--port=%d", universeGrpcPort),
			},
			port:      universeGrpcPort,
			buildPath: "cmake",
			buildArgs: []string{
				"--build", "./build",
				"--config", "Release",
				"--target", "universe_server",
			},
		},
		{
			name:  "frontend",
			color: 91,
			path:  "build/frontend/frontend_server",
			args: []string{
				fmt.Sprintf("--grpc-port=%d", frontendGrpcPort),
				"--grpcweb-port=6101",
				fmt.Sprintf("--universe-addr=:%d", universeGrpcPort),
				"--index-tmpl=src/client/index.tmpl",
				"--index-js=build/client/client.js",
			},
			port: frontendGrpcPort,
		},
	}
	jobsByName := make(map[string]*job)
	for _, job := range jobs {
		jobsByName[job.name] = job
	}

	formattedJobNames := strings.Join(slices.Collect(maps.Keys(jobsByName)), ", ")
	argJobName := ""
	for _, arg := range flag.Args() {
		jobName, isJobSection := strings.CutSuffix(arg, ":")
		if isJobSection {
			if _, ok := jobsByName[jobName]; !ok {
				log.Fatal("Unknown job name: '", jobName, "'. Allowed jobs: ", formattedJobNames)
			}
			argJobName = jobName
		} else if argJobName == "" {
			log.Fatal(
				"Unexpected argument before the first job qualifier: `", arg, "`. ",
				"Arguments after `--` need to specify the job they are referring to using a qualifier `[job_name]:`. ",
				"For example `-- frontend: foo bar universe: qux`. ",
				"Allowed jobs: ", formattedJobNames, ".",
			)
		} else {
			job := jobsByName[argJobName]
			job.args = append(job.args, arg)
		}
	}

	launcher := launcher{
		jobs: jobs,
	}
	err = launcher.repl()
	if err != nil {
		log.Error(err)
		os.Exit(1)
	}
}
