package main

import (
	"context"
	"log"
	"time"

	"github.com/mkacz91/spejs/pb"
	"google.golang.org/grpc"
)

type UniverseServiceServerImpl struct {
	pb.UnimplementedUniverseServiceServer
	backendConn *grpc.ClientConn
	backend     pb.UniverseServiceClient
}

func NewUniverseServiceServer(backendConn *grpc.ClientConn) *UniverseServiceServerImpl {
	backend := pb.NewUniverseServiceClient(backendConn)
	return &UniverseServiceServerImpl{
		backendConn: backendConn,
		backend:     backend,
	}
}

func (s *UniverseServiceServerImpl) OpticalSample(c context.Context, req *pb.OpticalSampleRequest) (*pb.OpticalSampleResponse, error) {
	return s.backend.OpticalSample(c, req)
}

func (s *UniverseServiceServerImpl) Ping(c context.Context, req *pb.PingRequest) (*pb.PingResponse, error) {
	return &pb.PingResponse{Message: "Go!"}, nil
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

type SkyboxServiceServer struct {
	pb.UnimplementedSkyboxServiceServer
	backend pb.SkyboxServiceClient
}

func (s *SkyboxServiceServer) Render(ctx context.Context, req *pb.SkyboxRenderRequest) (*pb.SkyboxRenderResponse, error) {
	return s.backend.Render(ctx, req)
}

type TaskServiceServer struct {
	pb.UnimplementedTaskServiceServer
	backend pb.TaskServiceClient
}

func (s *TaskServiceServer) Poll(ctx context.Context, req *pb.TaskPollRequest) (*pb.TaskPollResponse, error) {
	return s.backend.Poll(ctx, req)
}
