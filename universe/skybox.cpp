#include "skybox.h"

#include "task.h"

glm::vec3 vec3_from_proto(const google::protobuf::RepeatedField<float> &proto);

SkyboxServiceServer::SkyboxServiceServer(TaskQueue &tasks)
		: tasks(tasks) { }

grpc::Status SkyboxServiceServer::Render(grpc::ServerContext *ctx, const SkyboxRenderRequest *req, SkyboxRenderResponse *rsp) {
	glm::vec3 position = vec3_from_proto(req->position());
	tasks.add<SkyboxRenderTask>(position);
	return grpc::Status::OK;
}

SkyboxRenderTask::SkyboxRenderTask(glm::vec3 position)
		: Task(TaskType::kSkyboxRender)
		, position(position) { }

void SkyboxRenderTask::write_result(TaskResult *result) {
	result->mutable_skybox_render()->Swap(&this->result);
}

glm::vec3 vec3_from_proto(const google::protobuf::RepeatedField<float> &proto) {
	return proto.size() == 3
			? glm::vec3(proto[0], proto[1], proto[2])
			: glm::vec3(NAN, NAN, NAN);
}
