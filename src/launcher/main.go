package main

import (
	"flag"
	"fmt"
	"os"
	"time"
)

var (
	statusTimeout    = flag.Duration("status-timeout", 10*time.Second, "The timeout after which the launcher should stop trying to confirm that the RPC server has started or stopped.")
	universePort     = flag.Int("universe-port", 8100, "The port of the universe server")
	frontendWebPort  = flag.Int("frontend-web-port", 8000, "The web port of the frontend server")
	frontendGrpcPort = flag.Int("frontend-grpc-port", 8001, "The gRPC port of the frontend server")
)

func main() {
	flag.Parse()

	_, err := os.Stat("go.work")
	if err != nil {
		fmt.Println("This program must be run from the root of the repository.")
		os.Exit(1)
	}

	fmt.Println("==== Starting jobs")

	jobs := []*Job{
		NewJob(
			"build/src/universe/Release/universe_server",
			"--port", *universePort,
		),
		NewJob(
			"build/src/frontend/frontend_server",
			"--grpc-port", *frontendGrpcPort,
			fmt.Sprintf("--web-port=%d", *frontendWebPort),
			"--client-src-dir=src/client",
			"--client-dist-dir=build/src/client/dist",
		),
	}

	for _, job := range jobs {
		fmt.Println()
		err = job.Start(*statusTimeout)
		if err != nil {
			break
		}
	}

	fmt.Println()
	if err != nil {
		fmt.Println("Not all jobs started successfully, aborting.")
	} else {
		fmt.Println("Sleeping 20 seconds ... ")
		time.Sleep(60 * time.Second)
	}

	fmt.Println()
	fmt.Println("==== Stopping jobs")

	for i := len(jobs) - 1; i >= 0; i-- {
		job := jobs[i]
		if job.State() == Running {
			fmt.Println()
			job.Stop(*statusTimeout)
		}
	}
}
