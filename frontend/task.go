package main

import (
	"context"
	"fmt"
	"log"
	"sync"
	"sync/atomic"

	"github.com/mkatch/spejs/pb"
	"github.com/mkatch/spejs/universepb"
)

type TaskServiceServer struct {
	pb.UnimplementedTaskServiceServer
	mut          sync.Mutex
	backend      universepb.TaskServiceClient
	stream       universepb.TaskService_StreamClient
	cancelStream context.CancelFunc
	listeners    []pb.TaskService_ListenServer
	streamChan   chan error
	nextTaskId   atomic.Uint64 // TODO: Not enought for multiple frontends and restarting.
}

func (s *TaskServiceServer) StartStreaming(backend universepb.TaskServiceClient) (err error) {
	if s.backend != nil {
		return fmt.Errorf("already streaming")
	}

	ctx, cancelStream := context.WithCancel(context.Background())
	s.backend = backend
	s.stream, err = backend.Stream(ctx)
	if err != nil {
		s.stream = nil
		cancelStream()
		return err
	}
	s.cancelStream = cancelStream

	s.streamChan = make(chan error)
	go func() {
		defer close(s.streamChan)
		for {
			rsp, err := s.stream.Recv()
			log.Println("Received", rsp, err)
			if err != nil {
				s.streamChan <- err
				break
			}
			func() {
				s.mut.Lock()
				defer s.mut.Unlock()
				for _, l := range s.listeners {
					log.Println("Sending", rsp, "to", l)
					l.Send(rsp)
				}
			}()
		}
	}()

	return nil
}

func (s *TaskServiceServer) StopStreaming() error {
	s.cancelStream()
	return <-s.streamChan
}

func (s *TaskServiceServer) Schedule(ctx context.Context, req *pb.TaskScheduleRequest) (*pb.TaskScheduleResponse, error) {
	taskId := s.nextTaskId.Add(1)
	err := s.stream.Send(&universepb.TaskRequest{
		TaskId: taskId,
		Task:   req.Request,
	})
	if err != nil {
		return nil, err
	}
	return &pb.TaskScheduleResponse{TaskId: taskId}, nil
}

func (s *TaskServiceServer) Listen(req *pb.TaskListenRequest, rsp pb.TaskService_ListenServer) error {
	log.Println("Listen", req)
	s.mut.Lock()
	s.listeners = append(s.listeners, rsp)
	s.mut.Unlock()
	<-rsp.Context().Done()
	return nil
}
