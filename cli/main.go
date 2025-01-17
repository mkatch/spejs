package main

import (
	"fmt"
	"os"

	"github.com/mkatch/spejs/cli/proto"
)

var subCmds = map[string]func([]string) error{
	"fixup-js-proto": proto.FixupJsProto,
}

func main() {
	if len(os.Args) < 2 {
		printUsageAndDie()
	}
	subCmdName := os.Args[1]
	subCmdArgs := os.Args[2:]

	subCmd, ok := subCmds[subCmdName]
	if !ok {
		fmt.Printf("Unknown subcommand %s. Avaliable subcommands:\n", subCmdName)
		for name := range subCmds {
			fmt.Println(name)
		}
		os.Exit(1)
	}

	err := subCmd(subCmdArgs)
	if err != nil {
		fmt.Printf("Error: %v\n", err)
		os.Exit(1)
	}
}

func printUsageAndDie() {
	fmt.Printf("Usage: %s <subcommand> [args]\n", os.Args[0])
	fmt.Printf("Available subcommands:\n")
	for name := range subCmds {
		fmt.Printf("  %s\n", name)
	}
	os.Exit(1)
}
