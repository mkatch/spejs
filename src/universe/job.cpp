#include "job.h"

void JobServiceImpl::wait_for_quit() {
  _quit_requested.get_future().wait();
}

grpc::Status JobServiceImpl::Status(
    grpc::ServerContext *context,
    google::protobuf::Empty const *request,
    JobStatusResponse *response) {
  response->set_is_ready(true);
  return grpc::Status::OK;
}

grpc::Status JobServiceImpl::Quit(
    grpc::ServerContext *context,
    google::protobuf::Empty const *request,
    google::protobuf::Empty *response) {
  _quit_requested.set_value();
  return grpc::Status::OK;
}
