#include "universe.h"
#include "tasks.h"

grpc::Status UniverseServiceImpl::Ping(grpc::ServerContext *c, PingRequest const *req, PingResponse *rsp) {
	rsp->set_message("Pong " + req->message());
	return grpc::Status::OK;
}

grpc::Status UniverseServiceImpl::OpticalSample(grpc::ServerContext *c, OpticalSampleRequest const *req, OpticalSampleResponse *rsp) {
	rsp->set_width(100);
	rsp->set_height(100);
	rsp->set_pixels("I AM PIXELS!");
	return grpc::Status::OK;
};

grpc::Status UniverseServiceImpl::Skybox(grpc::ServerContext *c, const SkyboxRequest *req, SkyboxResponse *rsp) {
	string asset_id = "skybox.qoi";
	rsp->set_asset_id(asset_id);
	tasks.emplace(SkyboxTask{std::move(asset_id)});
	return grpc::Status::OK;
};