package main

import (
	"context"
	"fmt"
	"os"
	"os/exec"
	"runtime"
	"strings"
	"sync/atomic"
	"syscall"
	"time"
)

type job struct {
	name                  string
	color                 int
	path                  string
	args                  []string
	buildPath             string
	buildArgs             []string
	log                   logger
	service               JobService
	process               *os.Process
	exitState             *atomic.Pointer[jobExitState]
	exited                chan bool
	status                *JobStatus
	lastStatusRefreshTime time.Time
	warnings              []error
	errors                []error
}

type jobExitState struct {
	processState *os.ProcessState
	error        error
}

func (job *job) init() error {
	job.log = log.withPrefix(fmt.Sprintf("[\033[38;5;%dm%8s\033[0m] ", job.color, job.name))
	return nil
}

func (job *job) command() string {
	return job.path + " " + strings.Join(job.args, " ")
}

func (job *job) appendError(err error) error {
	job.errors = append(job.errors, err)
	return err
}

func (job *job) logAppendErrorf(format string, a ...any) error {
	return job.appendError(job.log.Errorf(format, a...))
}

func (job *job) clearErrorsAndWarnings() {
	job.errors = nil
	job.warnings = nil
}

func (job *job) describe() {
	var b strings.Builder
	b.WriteString("\n\n")
	if job.process == nil {
		b.WriteString("    Status: not attached\n")
	} else {
		if job.status == nil {
			b.WriteString("    Status: attached, unknown\n")
		} else if job.status.IsReady {
			b.WriteString("    Status: attached, ready\n")
		} else {
			b.WriteString("    Status: attached, not read\n")
		}
		b.WriteString(fmt.Sprintf("    PID:    %d\n", job.process.Pid))
	}
	b.WriteString(fmt.Sprintf("    Port:   %d\n", job.service.Port()))
	b.WriteString(fmt.Sprintf("    Command: %s\n", job.command()))
	for _, warn := range job.warnings {
		b.WriteString(fmt.Sprintf("    %s\n", formatWarningLog(warn)))
	}
	for _, err := range job.errors {
		b.WriteString(fmt.Sprintf("    %s\n", formatErrorLog(err)))
	}
	b.WriteString("\n")
	job.log.Print(b.String())
}

func (job *job) summary() string {
	var b strings.Builder
	b.WriteString(fmt.Sprintf("\033[38;5;%dm%s\033[0m", job.color, job.name))
	if len(job.warnings) > 0 {
		b.WriteString("*")
	}
	b.WriteString(" ")
	if len(job.errors) > 0 {
		b.WriteString("\033[38;5;1mX\033[0m")
	} else if job.status == nil {
		b.WriteString("\033[38;5;244m?\033[0m")
	} else if job.status.IsReady {
		b.WriteString("\033[38;5;2mO\033[0m")
	} else {
		b.WriteString("\033[38;5;3mN\033[0m")
	}
	return b.String()
}

func (job *job) attach(attemptCount int) (err error) {
	if job.process != nil {
		job.log.Print("Job already attached.")
		return nil
	}

	defer func() {
		if err != nil {
			job.stopIfRunningAndDetach()
		}
	}()

	var attach *JobAttach
	for attempt := 0; attach == nil && attempt < attemptCount; attempt++ {
		if attempt > 0 {
			time.Sleep(1 * time.Second)
		}
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		attach, err = job.service.Attach(ctx)
		cancel()
		if err != nil && attemptCount > 1 {
			job.log.Printf("Attach RPC attempt %d (out of %d) failed: %v", attempt, attemptCount, err)
		}
	}

	if err != nil {
		return
	}

	if attach.Command != job.command() {
		warn := job.log.Warningf("command different than expected:\n%s", attach.Command)
		job.warnings = append(job.warnings, warn)
	}

	job.process, err = findProcess(attach.Pid)
	if err != nil {
		return fmt.Errorf("can't find process: %w", err)
	}

	exitState := &atomic.Pointer[jobExitState]{}
	exited := make(chan bool)
	job.exitState = exitState
	job.exited = exited
	go func() {
		state, err := job.process.Wait()
		exitState.Store(&jobExitState{processState: state, error: err})
		exited <- true
		close(exited)
	}()

	job.log.Printf("Process attached. PID: %d", attach.Pid)
	job.refreshStatus()
	return
}

func (job *job) attachOrStart() error {
	if job.process != nil {
		job.log.Print("Job already attached")
		return nil
	}

	job.clearErrorsAndWarnings()
	job.log.Print("Trying to attach to an already running job...")
	err := job.attach(1)
	if err == nil {
		return nil
	}

	job.log.Printf("Unable to attach to an already running job: %v.", err)
	return job.start()
}

func (job *job) start() error {
	if job.process != nil {
		job.log.Print("Job already attached")
		return nil
	}

	job.clearErrorsAndWarnings()

	if job.buildPath != "" {
		job.log.Print("Building job...")
		cmd := exec.Command(job.buildPath, job.buildArgs...)
		writer := job.log.WrappedWriter()
		cmd.Stdout = writer
		cmd.Stderr = writer
		err := cmd.Run()
		if err != nil {
			return job.logAppendErrorf("build: %w", err)
		}
	}

	// Windows specific:
	wtArgs := append([]string{
		"--window=0",
		"new-tab",
		fmt.Sprintf("--title=%s", job.name),
		job.path,
	},
		job.args...,
	)
	cmd := exec.Command("wt", wtArgs...)
	job.log.Printf("Starting job...\n%s", strings.Join(cmd.Args, " "))
	err := cmd.Start()
	if err != nil {
		return job.logAppendErrorf("starting job: %w", err)
	}

	time.Sleep(1 * time.Second)
	job.log.Print("Job started. Attaching...")
	err = job.attach(5)
	if err != nil {
		return job.logAppendErrorf("attach: %w", err)
	}

	return nil
}

func (job *job) restartOrAttach() error {
	if job.process != nil {
		job.stopIfRunningAndDetach()
		return job.start()
	} else {
		return job.attachOrStart()
	}
}

func (job *job) stopIfRunningAndDetach() {
	if job.process != nil && job.exitState.Load() == nil {
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		defer cancel()

		job.log.Print("Sending Quit request to gracefully stop the job...")
		err := job.service.Quit(ctx)
		if err != nil {
			job.logAppendErrorf("quit: %w", err)
		} else {
			job.log.Print("Quit request accepted. Waiting for the process to terminate...")
			select {
			case <-ctx.Done():
				job.log.Print("Timed out waiting for process to terminate.")
				break
			case <-job.exited:
				break
			}
		}

		if job.exitState.Load() == nil {
			job.log.Print("Forcefully quitting with SIGKILL and waiting for it to terminate...")
			err = job.process.Signal(os.Kill)
			if err != nil {
				job.logAppendErrorf("SIGKILL: %w", err)
			} else {
				select {
				case <-time.After(5 * time.Second):
					job.log.Print("Timed out waiting for process to terminate.")
					break
				case <-job.exited:
					break
				}
			}
		}

		exitState := job.exitState.Load()
		if exitState != nil {
			ps := exitState.processState
			if ps != nil {
				job.log.Printf("Process PID %d exited with %d.", ps.Pid(), ps.ExitCode())
			}
			err := exitState.error
			if err != nil {
				job.logAppendErrorf("process wait: %w", err)
			}
		} else {
			job.logAppendErrorf("coudn't terminate process; abadonned")
		}
	}
	job.process = nil

	err := job.service.Close()
	if err != nil {
		job.logAppendErrorf("close connection: %w", err)
	}

	job.warnings = nil
	job.exitState = nil
	job.exited = nil
	job.status = nil
}

func (job *job) stop() error {
	if job.process == nil {
		return fmt.Errorf("job not attached")
	}
	job.stopIfRunningAndDetach()
	return nil
}

func (job *job) refreshStatus() {
	if job.process == nil {
		return
	}

	if job.exitState.Load() != nil {
		job.stopIfRunningAndDetach()
		job.logAppendErrorf("process exited unexpectedly")
		return
	}

	shouldRefresh :=
		job.status == nil || !job.status.IsReady ||
			time.Since(job.lastStatusRefreshTime) > 20*time.Second
	if !shouldRefresh {
		return
	}

	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	status, err := job.service.Status(ctx)

	if err != nil {
		job.status = nil
		job.log.Errorf("status: %w", err)
	} else {
		job.status = status
		job.lastStatusRefreshTime = time.Now()
	}
}

func findProcess(pid int) (*os.Process, error) {
	process, err := os.FindProcess(pid)
	if err == nil && runtime.GOOS != "windows" {
		// On Unix systems, `FindProcess` always returns a process handle and to
		// really check if the process exists, you need to send a signal.
		err = process.Signal(syscall.Signal(0))
	}
	if err != nil {
		return nil, err
	} else {
		return process, nil
	}
}
