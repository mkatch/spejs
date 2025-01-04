package main

import (
	"context"
	"fmt"

	"github.com/mkacz91/spejs/pb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

type grpcJobService struct {
	port   int
	conn   *grpc.ClientConn
	client pb.JobServiceClient
}

func (s *grpcJobService) Port() int {
	return s.port
}

func (s *grpcJobService) Attach(ctx context.Context) (*JobAttach, error) {
	if s.client == nil {
		conn, err := grpc.Dial(
			fmt.Sprintf("localhost:%d", s.port),
			grpc.WithTransportCredentials(insecure.NewCredentials()),
		)
		if err != nil {
			return nil, err
		}
		s.conn = conn
		s.client = pb.NewJobServiceClient(conn)
	}

	rsp, err := s.client.Attach(ctx, &pb.Empty{})
	if err != nil {
		return nil, err
	}

	return &JobAttach{
		Pid:     int(rsp.Pid),
		Command: rsp.Command,
	}, nil
}

func (s *grpcJobService) Status(ctx context.Context) (*JobStatus, error) {
	if s.client == nil {
		return nil, fmt.Errorf("not attached")
	}
	rsp, err := s.client.Status(ctx, &pb.Empty{})
	if err != nil {
		return nil, err
	}
	return &JobStatus{IsReady: rsp.IsReady}, nil
}

func (s *grpcJobService) Quit(ctx context.Context) error {
	if s.client == nil {
		return fmt.Errorf("not attached")
	}
	_, err := s.client.Quit(ctx, &pb.Empty{})
	return err
}

func (s *grpcJobService) Close() error {
	if s.conn != nil {
		s.client = nil
		return s.conn.Close()
	} else {
		return nil
	}
}
