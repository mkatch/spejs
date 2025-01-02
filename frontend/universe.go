package main

import (
	"context"
	"fmt"
	"log"
	"time"

	"github.com/mkacz91/spejs/pb"
	"github.com/mkacz91/spejs/universe"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

type UniverseServiceServerImpl struct {
	pb.UnimplementedUniverseServiceServer
	backendConn   *grpc.ClientConn
	backend       pb.UniverseServiceClient
	skyboxBackend universe.UniverseSkyboxServiceClient
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
	skyboxBacked := universe.NewUniverseSkyboxServiceClient(backendConn)
	return &UniverseServiceServerImpl{
		backendConn:   backendConn,
		backend:       backend,
		skyboxBackend: skyboxBacked,
	}, nil
}

func (s *UniverseServiceServerImpl) Close() {
	s.backendConn.Close()
}

func (s *UniverseServiceServerImpl) OpticalSample(c context.Context, req *pb.OpticalSampleRequest) (*pb.OpticalSampleResponse, error) {
	return s.backend.OpticalSample(c, req)
}

func (s *UniverseServiceServerImpl) Ping(c context.Context, req *pb.PingRequest) (*pb.PingResponse, error) {
	return &pb.PingResponse{Message: "Go!"}, nil
}

func (s *UniverseServiceServerImpl) Skybox(ctx context.Context, req *pb.SkyboxRequest) (*pb.SkyboxResponse, error) {
	assetId := "skybox.qoi"
	_, err := s.skyboxBackend.Render(ctx, &universe.UniverseSkyboxRenderRequest{
		DestPath: assetId,
	})
	if err == nil {
		return &pb.SkyboxResponse{AssetId: assetId}, nil
	} else {
		return nil, err
	}
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
