package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net"
	"net/http"
	"sync"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/improbable-eng/grpc-web/go/grpcweb"
	"github.com/mkacz91/spejs/pb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
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

	grpcLis, err := net.Listen("tcp4", fmt.Sprintf(":%d", *grpcPort))
	if err != nil {
		log.Fatal("Unable to listen on the gRPC port: ", err)
	}
	webLis, err := net.Listen("tcp4", fmt.Sprintf(":%d", *webPort))
	if err != nil {
		log.Fatal("Unable to listen on the web port: ", err)
	}

	wg := sync.WaitGroup{}

	grpcServer := grpc.NewServer()
	jobService := NewJobService()
	pb.RegisterJobServiceServer(grpcServer, jobService)
	grpcwebServer := grpcweb.WrapServer(
		grpcServer,
		grpcweb.WithAllowNonRootResource(true),
	)

	// wgrpcServer := grpc.NewServer()
	// wjobService := NewJobService()
	// pb.RegisterJobServiceServer(wgrpcServer, wjobService)

	log.Println("Starting gRPC server on ", grpcLis.Addr())
	wg.Add(1)
	go func() {
		err := grpcServer.Serve(grpcLis)
		if err != grpc.ErrServerStopped {
			log.Println("Failed to serve gRPC:", err)
		}
		wg.Done()
	}()

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
		jobService.WaitForQuit()
		grpcServer.GracefulStop()
		webServer.Shutdown(context.Background())
	}()

	go pingUniverse()

	go func() {
		jobService.WaitForQuit()
		grpcServer.GracefulStop()

	}()

	go pingUniverse()

	wg.Wait()
}

func pingUniverse() {
	conn, err := grpc.Dial(
		fmt.Sprintf("localhost:%d", *universePort),
		grpc.WithTransportCredentials(insecure.NewCredentials()),
	)
	if err != nil {
		log.Println("Failed to dial universe server:", err)
		return
	}
	defer conn.Close()

	universeClient := pb.NewUniverseServiceClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	response, err := universeClient.Ping(ctx, &pb.PingRequest{Message: "Go!"})
	if err != nil {
		log.Println("UniverseService.Ping request failed:", err)
		return
	}

	log.Printf("UniverseService.Ping response: %s", response.Message)
}
