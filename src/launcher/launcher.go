package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"sync"
	"time"
)

var (
	statusTimeout = flag.Duration("status-timeout", 10*time.Second, "The timeout after which the launcher should stop trying to confirm that the RPC server has started or stopped.")
)

type Launcher struct {
	jobs         []*Job
	cancelStart  context.CancelFunc
	wg           sync.WaitGroup
	startStopMut sync.Mutex
	stopOnce     sync.Once
}

func NewLauncher(jobs []*Job) *Launcher {
	return &Launcher{
		jobs: jobs,
	}
}

func (l *Launcher) StartAll() error {
	l.startStopMut.Lock()
	defer l.startStopMut.Unlock()
	if l.cancelStart != nil {
		return fmt.Errorf("already started")
	}
	startContext, cancelStart := context.WithCancel(context.Background())
	l.cancelStart = cancelStart
	defer cancelStart()
	l.wg.Add(1)

	fmt.Println("")
	fmt.Println("=== Starting jobs ")
	fmt.Println("")

	for _, job := range l.jobs {
		if startContext.Err() != nil {
			// Breaking here should not make a logical difference, as the callers are
			// still responsible for cleaning up all running jobs. This just expedites
			// the process of shutting down by not starting jobs destined for failure.
			return startContext.Err()
		}
		c, cancel := context.WithTimeout(startContext, *statusTimeout)
		defer cancel()

		fmt.Print("Starting ", job.Name(), " ... ")
		err := printOperationResult(job.Start(c))
		if err != nil {
			return err
		}
		fmt.Println("Logging to", job.LogFileName())

		l.wg.Add(1)
		go func() {
			defer l.wg.Done()
			job.Wait()
		}()

		fmt.Print("Waiting for ", job.Name(), " to be ready ... ")
		err = printOperationResult(job.WaitReady())
		if err != nil {
			return err
		}
	}

	return nil
}

func (l *Launcher) StopAll() error {
	if l.cancelStart == nil {
		return fmt.Errorf("not started")
	}
	l.stopOnce.Do(func() {
		l.cancelStart()
		l.startStopMut.Lock()
		defer l.startStopMut.Unlock()
		c, cancel := context.WithTimeout(context.Background(), *statusTimeout)
		defer cancel()

		for _, job := range l.jobs {
			fmt.Print("Asking ", job.Name(), " to stop ... ")
			printOperationResult(job.Stop(c))
		}

		for _, job := range l.jobs {
			fmt.Print("Waiting for ", job.Name(), " to stop ... ")
			printOperationResult(job.Wait())
		}

		l.wg.Done()
	})
	return nil
}

func (l *Launcher) Wait() {
	l.wg.Wait()
}

const (
	opOK   = "\033[32mOK\033[0m"
	opFAIL = "\033[31mFAIL\033[0m"
)

func printOperationResult(err error) error {
	if err != nil {
		fmt.Println(opFAIL)
		fmt.Println(" ", err)
	} else {
		fmt.Println(opOK)
	}
	os.Stdout.Sync()
	return err
}
