package main

import (
	"context"

	"github.com/mkacz91/spejs/pb"
)

type JobServiceImpl struct {
	pb.UnimplementedJobServiceServer
	quitRequested chan bool
}

func NewJobService() *JobServiceImpl {
	return &JobServiceImpl{
		quitRequested: make(chan bool),
	}
}

func (s *JobServiceImpl) Status(ctx context.Context, request *pb.Empty) (*pb.JobStatusResponse, error) {
	return &pb.JobStatusResponse{IsReady: true}, nil
}

func (s *JobServiceImpl) Quit(ctx context.Context, request *pb.Empty) (*pb.Empty, error) {
	s.quitRequested <- true
	return &pb.Empty{}, nil
}

func (s *JobServiceImpl) WaitForQuit() {
	<-s.quitRequested
}
