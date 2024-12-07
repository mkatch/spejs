package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"
	"strings"
	"time"
)

var (
	universePort     = flag.Int("universe-port", 8100, "The port of the universe server")
	frontendWebPort  = flag.Int("frontend-web-port", 8000, "The web port of the frontend server")
	frontendGrpcPort = flag.Int("frontend-grpc-port", 8001, "The gRPC port of the frontend server")
)

func main() {
	go func() {
		scanner := bufio.NewScanner(os.Stdin)
		for scanner.Scan() {
			fmt.Printf("stdin: %s\n", scanner.Text())
		}
	}()

	for {
		<-time.After(time.Second * 5)
		fmt.Println("Still running...")
	}

	flag.Parse()

	_, err := os.Stat("go.work")
	if err != nil {
		fmt.Println("This program must be run from the root of the repository.")
		os.Exit(1)
	}

	launcher := NewLauncher([]*Job{
		NewJob(
			"build/Release/universe_server",
			"--port", *universePort,
		),
		NewJob(
			"build/frontend/frontend_server",
			"--grpc-port", *frontendGrpcPort,
			fmt.Sprintf("--web-port=%d", *frontendWebPort),
			"--index-tmpl=src/client/index.tmpl",
			"--index-js=build/client/client.js",
		),
	})

	go func() {
		lines := bufio.NewScanner(os.Stdin)
	repl:
		for lines.Scan() {
			line := lines.Text()
			tokens := strings.Fields(line)
			if len(tokens) == 0 {
				continue
			}
			cmdname := tokens[0]
			switch cmdname {
			case "q":
				break repl
			default:
				fmt.Println("Unknown command:", cmdname)
			}
		}
		launcher.StopAll()
	}()

	err = launcher.StartAll()
	if err != nil {
		fmt.Println("")
		fmt.Println("Not all jobs started successfully, aborting", err)
		launcher.StopAll()
	}
	launcher.Wait()
}
