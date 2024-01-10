package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net"
	"net/http"
	"sync"

	"github.com/gin-gonic/gin"
	"github.com/improbable-eng/grpc-web/go/grpcweb"
	"github.com/mkacz91/spejs/pb"
	"google.golang.org/grpc"
)

// TODO: I don't like passing index.html and index.js as flags. Hardcoding also
// doesn't seem like a good idea. Maybe we should have some kind of install
// build step that copies files to one directory?
var (
	webPort      = flag.Int("web-port", 8000, "The port to server web content")
	indexTmpl    = flag.String("index-tmpl", "missing", "Path to the app .html file")
	indexJs      = flag.String("index-js", "missing", "Path to the app .js bundle")
	grpcPort     = flag.Int("grpc-port", 8001, "The port to serve gRPC requests")
	universePort = flag.Int("universe-port", 8100, "The port of the universe server")
)

func main() {
	flag.Parse()
	err := mainWithError()
	if err != nil {
		log.Fatal(err)
	}
}

func mainWithError() error {
	wg := sync.WaitGroup{}

	grpcLis, err := net.Listen("tcp4", fmt.Sprintf(":%d", *grpcPort))
	if err != nil {
		return fmt.Errorf("unable to listen on the gRPC port: %w", err)
	}
	defer grpcLis.Close()

	webLis, err := net.Listen("tcp4", fmt.Sprintf(":%d", *webPort))
	if err != nil {
		return fmt.Errorf("unable to listen on the web port: %w", err)
	}
	defer webLis.Close()

	grpcServer := grpc.NewServer()

	universeServiceServer, err := NewUniverseServiceServer(fmt.Sprintf(":%d", *universePort))
	if err != nil {
		return fmt.Errorf("unable to create UniverseService server: %w", err)
	}
	defer universeServiceServer.Close()
	pb.RegisterUniverseServiceServer(grpcServer, universeServiceServer)

	jobServiceServer := NewJobServiceServer()
	pb.RegisterJobServiceServer(grpcServer, jobServiceServer)

	log.Println("Starting gRPC server on ", grpcLis.Addr())
	wg.Add(1)
	go func() {
		err := grpcServer.Serve(grpcLis)
		if err != grpc.ErrServerStopped {
			log.Println("Failed to serve gRPC:", err)
		}
		wg.Done()
	}()

	grpcwebServer := grpcweb.WrapServer(
		grpcServer,
		grpcweb.WithAllowNonRootResource(true),
	)

	router := gin.Default()
	router.LoadHTMLFiles(*indexTmpl)
	router.GET("/", func(c *gin.Context) {
		c.HTML(http.StatusOK, "index.tmpl", gin.H{})
	})
	router.StaticFile("/index.js", *indexJs)
	router.POST("/rpc/*method", gin.WrapH(grpcwebServer))
	webServer := &http.Server{Handler: router}

	log.Println("Starting web server on ", webLis.Addr())
	wg.Add(1)
	go func() {
		err := webServer.Serve(webLis)
		if err != http.ErrServerClosed {
			log.Println("Failed to serve client:", err)
		}
		wg.Done()
	}()

	go func() {
		jobServiceServer.WaitForQuit()
		grpcServer.GracefulStop()
		webServer.Shutdown(context.Background())
	}()

	go universeServiceServer.PingBackend()

	wg.Wait()
	return nil
}
