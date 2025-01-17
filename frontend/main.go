package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net"
	"net/http"
	"os"
	"sync"

	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
	"github.com/improbable-eng/grpc-web/go/grpcweb"
	"github.com/mkatch/spejs/pb"
	"github.com/mkatch/spejs/universepb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

// TODO: I don't like passing index.html and index.js as flags. Hardcoding also
// doesn't seem like a good idea. Maybe we should have some kind of install
// build step that copies files to one directory?
var (
	devClientRedirect = flag.String("dev-client-redirect", "", "Redirect for the client app request. Useful when running a separate dev client (Vite)")
	webPort           = flag.Int("web-port", 8000, "The port to serve web content")
	indexTmpl         = flag.String("index-tmpl", "missing", "Path to the app .html file")
	indexJs           = flag.String("index-js", "missing", "Path to the app .js bundle")
	grpcPort          = flag.Int("grpc-port", 6100, "The port to serve gRPC requests")
	grpcwebPort       = flag.Int("grpcweb-port", 6101, "The port to serve gRPC-Web requests")
	universeAddr      = flag.String("universe-addr", "", "The address of the universe server")
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

	grpcwebLis, err := net.Listen("tcp4", fmt.Sprintf(":%d", *grpcwebPort))
	if err != nil {
		return fmt.Errorf("unable to listen on the gRPC-Web port: %w", err)
	}
	defer grpcwebLis.Close()

	webLis, err := net.Listen("tcp4", fmt.Sprintf(":%d", *webPort))
	if err != nil {
		return fmt.Errorf("unable to listen on the web port: %w", err)
	}
	defer webLis.Close()

	universeConn, err := grpc.Dial(
		*universeAddr,
		grpc.WithTransportCredentials(insecure.NewCredentials()),
	)
	if err != nil {
		return fmt.Errorf("failed to dial universe server at %s: %w", *universeAddr, err)
	}
	defer universeConn.Close()
	universeTaskServiceClient := universepb.NewTaskServiceClient(universeConn)

	grpcServer := grpc.NewServer()

	jobServiceServer := NewJobServiceServer()
	pb.RegisterJobServiceServer(grpcServer, jobServiceServer)

	taskServiceServer := TaskServiceServer{}
	pb.RegisterTaskServiceServer(grpcServer, &taskServiceServer)

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
	corsConfig := cors.DefaultConfig()
	corsConfig.AllowAllOrigins = true
	router.Use(cors.New(corsConfig))

	if *devClientRedirect != "" {
		router.GET("/", func(c *gin.Context) {
			c.Redirect(http.StatusTemporaryRedirect, *devClientRedirect)
		})
	} else {
		router.LoadHTMLFiles(*indexTmpl)
		router.GET("/", func(c *gin.Context) {
			c.HTML(http.StatusOK, "index.tmpl", gin.H{})
		})
		router.StaticFile("/index.js", *indexJs)
	}
	if wd, err := os.Getwd(); err == nil {
		router.Static("/static", wd)
	}
	// router.POST("/rpc/*method", gin.WrapH(grpcwebServer))
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

	log.Println("Starting task streaming...")
	taskServiceServer.StartStreaming(universeTaskServiceClient)

	grpcwebServer := &http.Server{Handler: grpcweb.WrapServer(
		grpcServer,
		grpcweb.WithOriginFunc(func(origin string) bool {
			return true
		}),
	)}
	log.Println("Starting gRPC-Web server on ", grpcwebLis.Addr())
	wg.Add(1)
	go func() {
		err := grpcwebServer.Serve(grpcwebLis)
		if err != http.ErrServerClosed {
			log.Println("Failed to serve gRPC-Web:", err)
		}
		wg.Done()
	}()

	go func() {
		jobServiceServer.WaitForQuit()
		if err := taskServiceServer.StopStreaming(); err != nil {
			log.Printf("StopStreaming: %v.", err)
		}
		grpcServer.GracefulStop()
		grpcwebServer.Shutdown(context.Background())
		webServer.Shutdown(context.Background())
	}()

	wg.Wait()
	return nil
}
