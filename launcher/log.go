package main

import (
	"fmt"
	"io"
	stdlog "log"
	"os"
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

func formatFatalLog(err error) string {
	return fmt.Sprintf("\033[38;5;1mFATAL:\033[0m %v", err)
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

func (l *logger) Fatalf(format string, a ...any) {
	l.Print(formatFatalLog(fmt.Errorf(format, a...)))
	os.Exit(1)
}

type wrappedLogWriter struct {
	log *logger
}

func (l *logger) WrappedWriter() io.Writer {
	return &wrappedLogWriter{l}
}

func (w *wrappedLogWriter) Write(p []byte) (n int, err error) {
	w.log.Print(string(p))
	return len(p), nil
}
