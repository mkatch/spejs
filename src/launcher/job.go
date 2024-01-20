package main

import (
	"context"
	"errors"
	"fmt"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"sync"
	"time"

	"github.com/mkacz91/spejs/pb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

// Job represents a process that implements a service.
//
// It can be used started and stopped multiple times, each time spawning a new
// process, but only one process is allowed at a time.
type Job struct {
	mut     sync.Mutex
	name    string
	path    string
	args    []string
	port    int
	logFile *os.File
	log     *log.Logger
	run     *jobRun
}

// jobRun is a part of Job that is reset each time a new process is started.
//
// The Job can be started and stopped multiple times, but a single run has
// a strict lifecycle and must be disposed of after stopping.
type jobRun struct {
	cmd         *exec.Cmd
	cmdEnded    chan struct{}
	port        int
	conn        func() (*grpc.ClientConn, error)
	service     func() (pb.JobServiceClient, error)
	waitReady   func() error
	cancelReady context.CancelFunc
	waitCmd     func() error
	waitCalled  bool
	stopOnce    sync.Once
	log         *log.Logger
}

// NewJob creates a new Job from the given command.
//
// Does not start the command. To start the command, call Start.
func NewJob(path string, portKey string, port int, arg ...string) *Job {
	return &Job{
		name: filepath.Base(path),
		path: path,
		args: append(arg, fmt.Sprintf("%s=%d", portKey, port)),
		port: port,
	}
}

// Name returns the human readable name of the job.
func (job *Job) Name() string {
	return job.name
}

// LogFileName returns the name of the file where the job's output is logged.
func (job *Job) LogFileName() string {
	return job.logFile.Name()
}

// Start starts a new process running the job's command.
//
// Returns immediately after starting the command. If successful, initiates
// a readines check in the provided context c, but that happens asynchronously.
// To wait for the job to be ready, call WaitReady.
//
// The job can be started and stopped multiple times, but only one active
// process is allowed at a time, and each call to Start needs a corresponding
// Wait before the job can be started again. The process may exit out of its own
// accord or be requested stop using Stop.
func (job *Job) Start(c context.Context) error {
	job.mut.Lock()
	defer job.mut.Unlock()
	if job.run != nil {
		return fmt.Errorf("job already/still running")
	}

	if job.logFile == nil {
		logFile, err := os.CreateTemp("", "spejs_"+job.name+"_OUT_")
		if err != nil {
			return fmt.Errorf("unable to create log file: %w", err)
		}
		job.logFile = logFile
		job.log = log.New(logFile, "[launcher] ", log.Ltime|log.Lmsgprefix)
	}

	cmd := exec.Command(job.path, job.args...)
	cmd.Stdout = job.logFile
	cmd.Stderr = job.logFile
	job.log.Println("Starting command:")
	job.log.Println("  ", cmd)
	err := cmd.Start()
	if err != nil {
		job.log.Println(err)
		return err
	}
	job.log.Println("Command started, PID", cmd.Process.Pid)

	run := &jobRun{
		cmd:  cmd,
		port: job.port,
		log:  job.log,
	}

	run.conn = sync.OnceValues(func() (*grpc.ClientConn, error) { return conn(run) })
	run.service = sync.OnceValues(func() (pb.JobServiceClient, error) { return service(run) })

	readyContext, cancelReady := context.WithCancel(c)
	run.waitReady = sync.OnceValue(func() error { return waitReady(run, readyContext) })
	run.cancelReady = cancelReady
	go run.waitReady()

	run.waitCmd = sync.OnceValue(func() error { return waitCmd(run) })
	run.cmdEnded = make(chan struct{})
	go run.waitCmd()

	job.run = run
	return nil
}

// WaitReady waits for the job to be ready.
//
// This method does not initiate the readiness check, as it is alredy done in
// Start, it merely waits for it to complete.
//
// Returns error if the job is not running or the readiness could not be
// confirmed before the context ended.
func (job *Job) WaitReady() error {
	run, err := job.currentRun()
	if err != nil {
		return err
	}
	return run.waitReady()
}

// Wait waits for the current process to exit and cleans up resources.
//
// Each successful Start needs a corresponding Wait before the job can be
// started again. Wait can be called only once for a process.
//
// Returns an error if the job has not been started, the process exited
// abnormally, or there was a prolem with resource cleanup. In either case, the
// process should not be running.
func (job *Job) Wait() error {
	job.mut.Lock()
	defer job.mut.Unlock()

	if job.run == nil {
		return fmt.Errorf("job not started")
	}
	if job.run.waitCalled {
		// Technically, we could allow multiple calls but it is extremely error
		// prone. In concurrent code, even two immediately succeeding calls
		// to Wait may refer to different processes, because the job could have
		// been started and stopped an arbitrary number of times in between.
		return fmt.Errorf("Wait already called")
	}

	job.mut.Unlock()
	defer func() {
		job.mut.Lock()
		job.run = nil
	}()

	return job.run.waitCmd()
}

// Stops requests the current process to stop gracefully.
//
// The request happens asynchornously in the provided context c. This method
// returns immediately and does not wait for the process to stop. Fails only if
// the job is not running. To wait for the process to stop, call Wait, which
// will also report if the process ended abnormally.
//
// If the process does not terminate before the context ends, it will be
// forcefully killed with a SIGKILL.
//
// Subsequent calls are allowed and have no effect.
func (job *Job) Stop(c context.Context) error {
	run, err := job.currentRun()
	if err != nil {
		return err
	}
	run.stopOnce.Do(func() { go stop(run, c) })
	return nil
}

func (job *Job) currentRun() (*jobRun, error) {
	job.mut.Lock()
	defer job.mut.Unlock()
	if job.run == nil {
		return nil, fmt.Errorf("job not running")
	} else {
		return job.run, nil
	}
}

func conn(run *jobRun) (*grpc.ClientConn, error) {
	dialTarget := fmt.Sprintf("localhost:%d", run.port)
	run.log.Println("Dialing gRPC endpoint ", dialTarget)
	conn, err := grpc.Dial(
		dialTarget,
		grpc.WithTransportCredentials(insecure.NewCredentials()),
	)
	if err != nil {
		run.log.Println("Unable to connect to gRPC endpoint:", err)
	}
	return conn, err
}

func service(run *jobRun) (pb.JobServiceClient, error) {
	conn, err := run.conn()
	if err != nil {
		return nil, err
	}
	return pb.NewJobServiceClient(conn), nil
}

func waitReady(run *jobRun, c context.Context) error {
	service, err := run.service()
	if err != nil {
		return err
	}
	defer run.cancelReady()

	attemptCount := 0
	for {
		time.Sleep(1 * time.Second)
		attemptCount++
		run.log.Println("Querying readiness. Attempt", attemptCount)
		rsp, err := service.Status(c, &pb.Empty{})
		if err != nil {
			run.log.Println("Readiness query failed:", err)
			if c.Err() != nil {
				return err
			}
		} else if !rsp.IsReady {
			run.log.Println("Job responded not ready")
		} else {
			run.log.Println("Job responded ready")
			return nil
		}
	}
}

func waitCmd(run *jobRun) error {
	errs := make([]error, 0, 2)
	appendErr := func(err error) {
		run.log.Println(err)
		errs = append(errs, err)
	}

	err := run.cmd.Wait()
	if err != nil {
		appendErr(fmt.Errorf("command: %w", err))
	}
	run.log.Println("Job exited with", run.cmd.ProcessState.ExitCode())

	close(run.cmdEnded)
	run.cancelReady()

	conn, _ := run.conn()
	if conn != nil {
		err = conn.Close()
		if err != nil {
			appendErr(fmt.Errorf("close gRPC connection: %w", err))
		}
	}

	return errors.Join(errs...)
}

func stop(run *jobRun, c context.Context) {
	run.log.Println("Stopping job...")

	service, err := run.service()
	if err != nil {
		run.log.Println("Cannot gracefully stop the job because dialing the service failed:", err)
	} else {
		run.log.Print("Sending JobService.Quit RPC to gracefully stop the job...")
		_, err := service.Quit(c, &pb.Empty{})
		if err != nil {
			run.log.Println("JobService.Quit RPC failed:", err)
		} else {
			run.log.Println("JobService.Quit RPC succeeded. Waiting for the process to terminate...")
			select {
			case <-run.cmdEnded:
				return
			case <-c.Done():
				run.log.Println("Process still running despite accepting the Quit RPC, and the context ended with:", c.Err())
			}
		}
	}

	run.log.Print("Forcefully quitting process with a SIGKILL.")
	err = run.cmd.Process.Signal(os.Kill)
	if err != nil {
		// This should actually never happen, unless the process already died,
		// right? Log and forget for now, but consider propagating to Wait if there
		// is ever a need.
		run.log.Println("Unable to send SIGKILL:", err)
	}
}
