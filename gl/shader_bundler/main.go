package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"path"
	"strings"
	"time"
)

var invocationTime = time.Now().Format("2006-01-02 15:04:05 -0700")

func main() {
	subcmd := os.Args[1]
	output := os.Args[2]
	inputs := os.Args[3:]
	switch subcmd {
	case "cppheader":
		subCmdCppHeader(output, inputs)
	case "cppsrc":
		subCmdCppSrc(output, inputs)
	default:
		panic("Unknown subcommand " + subcmd)
	}
}

type ShaderSourceInfo struct {
	name        string
	file        string
	cppStruct   string
	cppTypeEnum string
}

func inferShaderSourceInfo(filePath string) ShaderSourceInfo {
	info := ShaderSourceInfo{
		name: fileStem(filePath),
		file: filePath,
	}
	if strings.HasSuffix(info.name, "_v") {
		info.cppStruct = "::gl::VertexShaderSource"
		info.cppTypeEnum = "GL_VERTEX_SHADER"
	} else if strings.HasSuffix(info.name, "_f") {
		info.cppStruct = "::gl::FragmentShaderSource"
		info.cppTypeEnum = "GL_FRAGMENT_SHADER"
	} else {
		panic("Shader file name must end with _v or _f to indicate shader type. Was " + info.name + ".")
	}
	return info
}

// The cppheader subcommand. Outputs the .h file declaring all the constants for
// individual shaders.
func subCmdCppHeader(header string, inputs []string) {
	out, err := os.Create(header)
	if err != nil {
		panic(err)
	}
	defer out.Close()
	fmt.Fprintf(out, "#pragma once\n")
	fmt.Fprintln(out)
	fmt.Fprintf(out, "// Generated file. DO NOT EDIT.\n")
	fmt.Fprintf(out, "// Time: %s\n", invocationTime)
	fmt.Fprintln(out)
	fmt.Fprintf(out, "#include <glpp/gl.h>\n")
	fmt.Fprintln(out)
	fmt.Fprintf(out, "struct ShaderSources {\n")
	for _, input := range inputs {
		fmt.Fprintln(out)
		info := inferShaderSourceInfo(input)
		fmt.Fprintf(out, "\t// %s\n", info.file)
		fmt.Fprintf(out, "\tstatic const %s %s;\n", info.cppStruct, info.name)
	}
	fmt.Fprintf(out, "};\n")
}

// The cppsrc subcommand. Outputs multiple .cpp files, one for each .glsl.
func subCmdCppSrc(header string, inputs []string) {
	outDir := path.Dir(header)
	header = path.Base(header)

	for _, filePath := range inputs {
		func() {
			info := inferShaderSourceInfo(filePath)
			input, err := os.Open(filePath)
			if err != nil {
				panic(err)
			}
			defer input.Close()
			lines := bufio.NewScanner(input)
			out, err := os.Create(path.Join(outDir, info.name+".cpp"))
			if err != nil {
				panic(err)
			}
			defer out.Close()
			outputCppSrcPart(lines, &info, out, header)
		}()
	}
}

// Outputs a .cpp file for a single .glsl.
func outputCppSrcPart(lines *bufio.Scanner, info *ShaderSourceInfo, out io.Writer, header string) {
	fmt.Fprintf(out, "// Generated file. DO NOT EDIT.\n")
	fmt.Fprintf(out, "// Original file: %s\n", info.file)
	fmt.Fprintf(out, "// Time: %s\n", invocationTime)
	fmt.Fprintln(out)
	fmt.Fprintf(out, "#include \"%s\"\n", header)
	fmt.Fprintln(out)
	fmt.Fprintf(out, "const %s ShaderSources::%s = {\n", info.cppStruct, info.name)
	fmt.Fprintf(out, "\t/* shader_type: */ %s,\n", info.cppTypeEnum)
	fmt.Fprintf(out, "\t/* name: */ \"%s\",\n", info.name)
	fmt.Fprintf(out, "\t/* file: */ \"%s\",\n", info.file)
	fmt.Fprintf(out, "\t/* source: */\n")
	for lines.Scan() {
		fmt.Fprintf(out, "\t\"%s\\n\"\n", lines.Text())
	}
	fmt.Fprintf(out, "};\n")
}

func fileStem(filePath string) string {
	return path.Base(strings.TrimSuffix(filePath, path.Ext(filePath)))
}
