package main

import (
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
)

type restJobService struct {
	port int
}

func (s *restJobService) Port() int {
	return s.port
}

func (s *restJobService) Attach(ctx context.Context) (*JobAttach, error) {
	var attach JobAttach
	err := s.getJson(ctx, "attach.json", &attach)
	if err != nil {
		return nil, err
	} else {
		return &attach, nil
	}
}

func (s *restJobService) Status(ctx context.Context) (*JobStatus, error) {
	var status JobStatus
	err := s.getJson(ctx, "status.json", &status)
	if err != nil {
		return nil, err
	} else {
		return &status, nil
	}
}

func (s *restJobService) Quit(ctx context.Context) error {
	_, err := s.request(ctx, "POST", "quit")
	return err
}

func (s *restJobService) Close() error {
	return nil
}

func (s *restJobService) request(ctx context.Context, method string, path string) (*http.Response, error) {
	url := fmt.Sprintf("http://localhost:%d/job/%s", s.port, path)
	req, err := http.NewRequestWithContext(ctx, method, url, nil)
	if err != nil {
		return nil, err
	}
	rsp, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, err
	}
	if rsp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed with status: %s (only 200 is accepted)", rsp.Status)
	}
	return rsp, nil
}

func (s *restJobService) getJson(ctx context.Context, path string, v any) error {
	rsp, err := s.request(ctx, "GET", path)
	if err != nil {
		return err
	}
	body, err := io.ReadAll(rsp.Body)
	if err != nil {
		return err
	}
	return json.Unmarshal(body, v)
}
