package main

import (
	"flag"
	"fmt"
	"maps"
	"os"
	"os/exec"
	"path/filepath"
	"slices"
	"strings"
)

func main() {
	initLog()
	flag.Parse()

	_, err := os.Stat("go.work")
	if err != nil {
		log.Fatalf("This program must be run from the root of the repository.")
	}

	universeGrpcPort := 6200
	frontendGrpcPort := 6100
	vitePort := 5173

	jobs := []*job{
		{
			name:  "universe",
			color: 214,
			path:  lookAbsPath("./build/Release/universe_server"),
			args: []string{
				fmt.Sprintf("--port=%d", universeGrpcPort),
			},
			buildPath: "cmake",
			buildArgs: []string{
				"--build", "./build",
				"--config", "Release",
				"--target", "universe_server",
			},
			service: &grpcJobService{
				port: universeGrpcPort,
			},
		},
		{
			name:  "frontend",
			color: 24,
			path:  lookAbsPath("./build/frontend/frontend_server"),
			args: []string{
				fmt.Sprintf("--grpc-port=%d", frontendGrpcPort),
				"--grpcweb-port=6101",
				fmt.Sprintf("--universe-addr=:%d", universeGrpcPort),
				fmt.Sprintf("--dev-client-redirect=http://localhost:%d/client/", vitePort),
			},
			buildPath: "cmake",
			buildArgs: []string{
				"--build", "./build",
				"--config", "Release",
				"--target", "frontend_server",
			},
			service: &grpcJobService{
				port: frontendGrpcPort,
			},
		},
		{
			name:  "vite",
			color: 98,
			path:  lookAbsPath("node"),
			args: []string{
				lookAbsPath("./node_modules/vite/bin/vite.js"),
				fmt.Sprintf("--port=%d", vitePort),
			},
			service: &restJobService{
				port: vitePort,
			},
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

func lookAbsPath(path string) string {
	var err error
	if resolvedPath, err := exec.LookPath(path); err == nil {
		if absPath, err := filepath.Abs(resolvedPath); err == nil {
			return absPath
		}
	}
	panic(fmt.Sprintf("Failed to find absolute path for: %s. Error: %v.", path, err))
}
