package main

import (
	"context"
	"os"
	"strings"

	"github.com/mkacz91/spejs/pb"
	"google.golang.org/protobuf/types/known/emptypb"
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

func (s *JobServiceServerImpl) Attach(ctx context.Context, req *emptypb.Empty) (*pb.JobAttachResponse, error) {
	return &pb.JobAttachResponse{
		Command: strings.Join(os.Args, " "),
		Pid:     int32(os.Getpid()),
	}, nil
}

func (s *JobServiceServerImpl) Status(ctx context.Context, req *emptypb.Empty) (*pb.JobStatusResponse, error) {
	return &pb.JobStatusResponse{IsReady: true}, nil
}

func (s *JobServiceServerImpl) Quit(ctx context.Context, req *emptypb.Empty) (*emptypb.Empty, error) {
	s.quitRequested <- true
	return &emptypb.Empty{}, nil
}

func (s *JobServiceServerImpl) WaitForQuit() {
	<-s.quitRequested
}
