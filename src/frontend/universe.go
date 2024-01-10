package main

import (
	"context"
	"fmt"
	"log"
	"time"

	"github.com/mkacz91/spejs/pb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

type UniverseServiceServerImpl struct {
	pb.UnimplementedUniverseServiceServer
	backendConn *grpc.ClientConn
	backend     pb.UniverseServiceClient
}

func NewUniverseServiceServer(backendAddr string) (*UniverseServiceServerImpl, error) {
	backendConn, err := grpc.Dial(
		backendAddr,
		grpc.WithTransportCredentials(insecure.NewCredentials()),
	)
	if err != nil {
		return nil, fmt.Errorf("failed to dial universe server at %s: %w", backendAddr, err)
	}
	backend := pb.NewUniverseServiceClient(backendConn)
	return &UniverseServiceServerImpl{
		backendConn: backendConn,
		backend:     backend,
	}, nil
}

func (s *UniverseServiceServerImpl) Close() {
	s.backendConn.Close()
}

func (s *UniverseServiceServerImpl) OpticalSample(c context.Context, req *pb.OpticalSampleRequest) (*pb.OpticalSampleResponse, error) {
	return s.backend.OpticalSample(c, req)
}

func (s *UniverseServiceServerImpl) PingBackend() {
	c, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()
	log.Println("Pinging universe backend")
	rsp, err := s.backend.Ping(c, &pb.PingRequest{Message: "Go!"})
	if err != nil {
		log.Println("Failed to ping universe backend:", err)
	} else {
		log.Println("Universe backend ping response:", rsp.Message)
	}
}
