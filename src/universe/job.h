#pragma once

#include <grpcpp/grpcpp.h>
#include <future>

#include "proto/job.grpc.pb.h"

class JobServiceImpl final : public JobService::Service {
public:
  void wait_for_quit();

  grpc::Status Status(
      grpc::ServerContext *context,
      Empty const *request,
      JobStatusResponse *response) override;

  grpc::Status Quit(
      grpc::ServerContext *context,
      Empty const *request,
      Empty *response) override;

private:
  std::promise<void> _quit_requested;
};