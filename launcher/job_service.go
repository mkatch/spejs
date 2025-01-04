package main

import "context"

type JobAttach struct {
	Pid     int    `json:"pid"`
	Command string `json:"command"`
}

type JobStatus struct {
	IsReady bool `json:"ready"`
}

type JobService interface {
	Attach(context.Context) (*JobAttach, error)
	Status(context.Context) (*JobStatus, error)
	Quit(context.Context) error
	Close() error
	Port() int
}
