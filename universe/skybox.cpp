#include "skybox.h"

#include "tasks.h"

UniverseSkyboxServiceServer::UniverseSkyboxServiceServer(TaskQueue &tasks)
		: tasks(tasks) { }

grpc::Status UniverseSkyboxServiceServer::Render(grpc::ServerContext *ctx, const UniverseSkyboxRenderRequest *req, UniverseSkyboxRenderResponse *rsp) {
	tasks.emplace(SkyboxTask{req->dest_path()});
	return grpc::Status::OK;
}
