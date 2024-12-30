package main

import (
	"fmt"
	stdlog "log"
)

type logger struct {
	*stdlog.Logger
}

var log = logger{stdlog.Default()}

func initLog() {
	log.SetPrefix("[\033[38;5;244mlauncher\033[0m] ")
	log.SetFlags(stdlog.LstdFlags | stdlog.Lmsgprefix)
}

func (l logger) withPrefix(prefix string) logger {
	return logger{stdlog.New(l.Writer(), prefix, l.Flags())}
}

func formatWarningLog(err error) string {
	return fmt.Sprintf("\033[38;5;3mWarning:\033[0m %v", err)
}

func formatErrorLog(err error) string {
	return fmt.Sprintf("\033[38;5;1mError:\033[0m %v", err)
}

func (l logger) Warning(err error) error {
	l.Print(formatWarningLog(err))
	return err
}

func (l logger) Warningf(format string, a ...any) error {
	return l.Warning(fmt.Errorf(format, a...))
}

func (l logger) Error(err error) error {
	l.Print(formatErrorLog(err))
	return err
}

func (l *logger) Errorf(format string, a ...any) error {
	return l.Error(fmt.Errorf(format, a...))
}
