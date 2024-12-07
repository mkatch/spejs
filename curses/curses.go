package main

import (
	"fmt"
	"os"

	"github.com/containerd/console"
	"golang.org/x/crypto/ssh/terminal"
	"golang.org/x/sys/windows"
)

func main() {
	current := console.Current()
	fmt.Println("current:", windows.Handle(current.Fd()))
	fmt.Println("stdin:", windows.Handle(os.Stdin.Fd()))
	defer current.Reset()

	if err := current.SetRaw(); err != nil {
		panic(err)
	}

	term := terminal.NewTerminal(current, "")
	term.AutoCompleteCallback = func(line string, pos int, key rune) (newLine string, newPos int, ok bool) {
		fmt.Println("callback:", line, pos, key)

		return "", 0, false
	}

	buf := make([]byte, 5)
	var n uint32
	err := windows.ReadFile(windows.Handle(current.Fd()), buf, &n, nil)

	fmt.Println("result:", buf, err)
}

// package main

// import (
// 	"fmt"
// 	"os"

// 	"golang.org/x/sys/windows"
// )

// func main() {
// 	fd := int(os.Stdin.Fd())
// 	var st uint32
// 	if err := windows.GetConsoleMode(windows.Handle(fd), &st); err != nil {
// 		panic(err)
// 	}
// 	fmt.Printf("st: %b\n", st)

// 	var raw = st
// 	raw &^= windows.ENABLE_ECHO_INPUT
// 	raw &^= windows.ENABLE_LINE_INPUT
// 	raw &^= windows.ENABLE_MOUSE_INPUT
// 	raw &^= windows.ENABLE_WINDOW_INPUT
// 	raw &^= windows.ENABLE_PROCESSED_INPUT

// 	// Enable these modes
// 	raw |= windows.ENABLE_EXTENDED_FLAGS
// 	raw |= windows.ENABLE_INSERT_MODE
// 	raw |= windows.ENABLE_QUICK_EDIT_MODE

// 	if err := windows.SetConsoleMode(windows.Handle(fd), raw); err != nil {
// 		panic(err)
// 	}
// 	fmt.Printf("raw: %b\n", raw)

// 	out := windows.Handle(int(os.Stdout.Fd()))
// 	var outMode uint32
// 	windows.GetConsoleMode(out, &outMode)
// 	windows.SetConsoleMode(out, outMode|windows.DISABLE_NEWLINE_AUTO_RETURN)

// 	fmt.Println("Press any key to exit...")

// 	buf := make([]byte, 1024)
// 	var n uint32
// 	err := windows.ReadFile(windows.Handle(os.Stdin.Fd()), buf, &n, nil)

// 	fmt.Println("result:", buf, err)

// 	// go func() {
// 	// 	scanner := bufio.NewScanner(os.Stdin)
// 	// 	for scanner.Scan() {
// 	// 		fmt.Printf("stdin: %s\n", scanner.Text())
// 	// 	}
// 	// }()

// 	// for {
// 	// 	<-time.After(time.Second * 5)
// 	// 	fmt.Println("Still running...")
// 	// }
// }
