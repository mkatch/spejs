package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"sync"
	"time"
)

type launcher struct {
	jobs                  []*job
	allJobIndices         []int
	lastStatus            string
	lastStatusRefreshTime time.Time
}

func (l *launcher) repl() error {
	for i, job := range l.jobs {
		err := job.init()
		if err != nil {
			return err
		}
		l.allJobIndices = append(l.allJobIndices, i)
	}

	l.attachOrStart(l.allJobIndices...)
	l.describeAll()

	line := make(chan string)
	go func() {
		lines := bufio.NewScanner(os.Stdin)
		for lines.Scan() {
			line <- lines.Text()
		}
	}()

	l.printStatus()

repl:
	for {
		select {
		case line := <-line:
			if l.userCommand(line) {
				break repl
			} else {
				l.lastStatusRefreshTime = time.Time{}
			}
		case <-time.After(5 * time.Second):
			l.printStatus()
		}
	}

	return nil
}

func (l *launcher) userCommand(line string) (quit bool) {
	quit = false
	fields := strings.Fields(line)
	if len(fields) == 0 {
		return
	}
	switch fields[0] {
	case "h":
		log.Print(`

    h          Show this help message.
    q          Quit all jobs and exit.
    d          Describe all jobs.
    d <index>  Describe job with the given index.

`)
		return
	case "q":
		if len(fields) == 1 {
			l.stopAll()
			return true
		}
	case "d":
		if len(fields) == 1 {
			l.describeAll()
			return
		} else if len(fields) == 2 {
			if i, err := strconv.ParseInt(fields[1], 0, 0); err == nil {
				if err = l.describe(int(i)); err == nil {
					return
				}
			}
		}
	}
	log.Print("Unknown command. Type 'h' for help.")
	return
}

func (l *launcher) printStatus() {
	l.eachJobParallel(func(job *job) error {
		job.refreshStatus()
		return nil
	}, l.allJobIndices...)

	var parts []string
	for i, job := range l.jobs {
		part := job.summary()
		parts = append(parts, fmt.Sprintf("%d: %s", i, part))
	}

	status := strings.Join(parts, ", ")
	if status != l.lastStatus || time.Since(l.lastStatusRefreshTime) > 1*time.Minute {
		l.lastStatus = status
		l.lastStatusRefreshTime = time.Now()
		log.Print(status)
	}
}

func (l *launcher) job(i int) (*job, error) {
	if i < 0 || len(l.jobs) <= i {
		return nil, fmt.Errorf("invalid job index: %d", i)
	} else {
		return l.jobs[i], nil
	}
}

func (l *launcher) eachJob(fn func(job *job) error, indices ...int) error {
	var errs []error
	for _, i := range indices {
		job, err := l.job(i)
		if err != nil {
			errs = append(errs, err)
			continue
		}
		err = fn(job)
		if err != nil {
			errs = append(errs, err)
		}
	}
	return errors.Join(errs...)
}

func (l *launcher) eachJobParallel(fn func(job *job) error, indices ...int) error {
	var errs []error
	var mut sync.Mutex
	appendErr := func(err error) {
		mut.Lock()
		defer mut.Unlock()
		errs = append(errs, err)
	}

	var wg sync.WaitGroup

	for i := range indices {
		job, err := l.job(i)
		if err != nil {
			appendErr(err)
			continue
		}

		wg.Add(1)
		go func() {
			err := fn(job)
			if err != nil {
				appendErr(err)
			}
			wg.Done()
		}()
	}
	wg.Wait()

	return errors.Join(errs...)
}

func (l *launcher) describe(indices ...int) error {
	return l.eachJob(func(job *job) error {
		job.describe()
		return nil
	}, indices...)
}

func (l *launcher) describeAll() error {
	return l.describe(l.allJobIndices...)
}

func focusThisTerminalTab() {
	cmd := exec.Command("wt", "--window=0", "focus-tab", "--target=0")
	err := cmd.Start()
	if err != nil {
		log.Printf("focus %v", err)
	}
}

func (l *launcher) attachOrStart(indices ...int) (err error) {
	err = l.eachJobParallel(func(job *job) error {
		return job.attachOrStart()
	}, indices...)
	focusThisTerminalTab()
	return
}

func (l *launcher) stop(indices ...int) error {
	return l.eachJobParallel(func(job *job) error {
		return job.stop()
	}, indices...)
}

func (l *launcher) stopAll() error {
	return l.stop(l.allJobIndices...)
}
