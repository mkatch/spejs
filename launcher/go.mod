module github.com/mkacz91/spejs/launcher

go 1.23.4

replace github.com/mkacz91/spejs/pb => ../build/proto/go

require (
	github.com/mkacz91/spejs/pb v0.0.0
	google.golang.org/grpc v1.60.1
	google.golang.org/protobuf v1.36.1
)

require (
	github.com/golang/protobuf v1.5.3 // indirect
	golang.org/x/net v0.16.0 // indirect
	golang.org/x/sys v0.13.0 // indirect
	golang.org/x/text v0.13.0 // indirect
	google.golang.org/genproto/googleapis/rpc v0.0.0-20231002182017-d307bd883b97 // indirect
)
