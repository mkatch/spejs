package main

import (
	"flag"
	"fmt"
	"time"
)

var (
	statusTimeout = flag.Duration("status_timeout", 10*time.Second,
		"The timeout after which the launcher should stop trying to "+
			"confirm that the RPC server has started or stopped.",
	)
	universePort = flag.Int(
		"universe_port", 8001, "The port of the universe server")
	frontendPort = flag.Int(
		"frontend_port", 8000, "The port of the frontend server")
)

func main() {
	flag.Parse()

	fmt.Println("==== Starting jobs")

	jobs := []*Job{
		NewJob("../../build/src/universe/Release/universe_server", *universePort),
		NewJob("../../build/src/frontend/frontend_server", *frontendPort),
	}

	var err error
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
		fmt.Println("Sleeping 10 seconds ... ")
		time.Sleep(10 * time.Second)
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
