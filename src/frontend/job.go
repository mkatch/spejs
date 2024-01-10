package main

import (
	"context"

	"github.com/mkacz91/spejs/pb"
)

type JobServiceServerImpl struct {
	pb.UnimplementedJobServiceServer
	quitRequested chan bool
}

func NewJobServiceServer() *JobServiceServerImpl {
	return &JobServiceServerImpl{
		quitRequested: make(chan bool),
	}
}

func (s *JobServiceServerImpl) Status(ctx context.Context, request *pb.Empty) (*pb.JobStatusResponse, error) {
	return &pb.JobStatusResponse{IsReady: true}, nil
}

func (s *JobServiceServerImpl) Quit(ctx context.Context, request *pb.Empty) (*pb.Empty, error) {
	s.quitRequested <- true
	return &pb.Empty{}, nil
}

func (s *JobServiceServerImpl) WaitForQuit() {
	<-s.quitRequested
}
