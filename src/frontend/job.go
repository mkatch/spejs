package main

import (
	"context"

	"github.com/golang/protobuf/ptypes/empty"
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

func (s *JobServiceImpl) Status(ctx context.Context, request *empty.Empty) (*pb.JobStatusResponse, error) {
	return &pb.JobStatusResponse{IsReady: true}, nil
}

func (s *JobServiceImpl) Quit(ctx context.Context, request *empty.Empty) (*empty.Empty, error) {
	s.quitRequested <- true
	return &empty.Empty{}, nil
}

func (s *JobServiceImpl) WaitForQuit() {
	<-s.quitRequested
}
