#pragma once

#include <grpcpp/grpcpp.h>

#include "proto/universe.grpc.pb.h"

class UniverseServiceImpl final : public UniverseService::Service {
public:
	grpc::Status Ping(grpc::ServerContext *c, PingRequest const *req, PingResponse *rsp) override;
	grpc::Status OpticalSample(grpc::ServerContext *c, OpticalSampleRequest const *req, OpticalSampleResponse *rsp) override;
};