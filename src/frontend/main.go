package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net"
	"time"

	"github.com/mkacz91/spejs/pb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

var (
	port         = flag.Int("port", 8000, "The port of the frontend server")
	universePort = flag.Int("universe_port", 8001, "The port of the universe server")
)

func main() {
	flag.Parse()
	lis, err := net.Listen("tcp4", fmt.Sprintf(":%d", *port))
	if err != nil {
		log.Fatal("Failed to listen: ", err)
	}

	server := grpc.NewServer()
	jobService := NewJobService()
	pb.RegisterJobServiceServer(server, jobService)
	log.Println("Starting frontend server on address", lis.Addr())

	go func() {
		jobService.WaitForQuit()
		server.Stop()
	}()

	go pingUniverse()

	err = server.Serve(lis)
	if err != nil {
		log.Fatal("Failed to serve: ", err)
	}
}

func pingUniverse() {
	conn, err := grpc.Dial(
		fmt.Sprintf("localhost:%d", *universePort),
		grpc.WithTransportCredentials(insecure.NewCredentials()),
	)
	if err != nil {
		log.Fatal("Failed to dial universe server: ", err)
	}
	defer conn.Close()

	universeClient := pb.NewUniverseServiceClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	response, err := universeClient.Ping(ctx, &pb.PingRequest{Message: "Go!"})
	if err != nil {
		log.Fatal("UniverseService.Ping request failed: ", err)
	}

	log.Printf("UniverseService.Ping response: %s", response.Message)
}
