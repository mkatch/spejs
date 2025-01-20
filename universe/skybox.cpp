#include <gl_cpp/gl.h>
#include <vector>

#include "shaders.h"
#include "skybox.h"

struct SkyboxVertex {
	glm::vec2 screen_position;
	glm::vec3 view_position;

	SkyboxVertex() = default;
	SkyboxVertex(int x, int y)
			: screen_position(x, y), view_position(x, y, -1) { }
};

class _Skybox final : public Skybox {
	static constexpr int N = 1024;

	const Shaders &shaders;
	gl::Texture3D texture = {GL_R8};
	gl::VertexBuffer<SkyboxVertex> vertex_buffer;
	GLuint vertex_array;
	GLsizei vertex_count;

	// gl::VertexArray vertex_array;

public:
	_Skybox(const Shaders &shaders)
			: shaders(shaders) {
		{
			std::cout << "Generating skybox texture..." << std::endl;
			int progress = -1;
			std::vector<uint8_t> data(N * N * N);
			for (int i = 0; i < N; i++) {
				// float x = (float)i / (float)(N - 1) * 2.0f - 1.0f;
				int jj = i * N * N;
				for (int j = 0; j < N; j++) {
					// float y = (float)j / (float)(N - 1) * 2.0f - 1.0f;
					int kk = jj + j * N;
					for (int k = 0; k < N; k++) {
						// float z = (float)k / (float)(N - 1) * 2.0f - 1.0f;
						data[kk + k] = i ^ j ^ k;
					}
				}
				int current_progress = (i + 1) * 10 / N;
				if (current_progress != progress) {
					progress = current_progress;
					std::cout << "Progress: " << progress * 10 << "%" << std::endl;
				}
			}

			texture.resize(N, N, N);
			texture.image(gl::RED8, data);
		}

		{
			std::cout << "Generating frustum vertices..." << std::endl;
			std::array<SkyboxVertex, 6> sheet{
				SkyboxVertex(-1, -1),
				SkyboxVertex(+1, -1),
				SkyboxVertex(+1, +1),
				SkyboxVertex(-1, -1),
				SkyboxVertex(+1, +1),
				SkyboxVertex(-1, +1),
			};
			std::vector<SkyboxVertex> vertices(6 * N);
			int progress = -1;
			for (int i = 0; i < N; ++i) {
				float f = (float)(i + 1) / (float)N;
				int jj = i * 6;
				for (int j = 0; j < 6; ++j) {
					SkyboxVertex &v = vertices[jj + j] = sheet[j];
					v.view_position *= f;
				}
				int current_progress = (i + 1) * 10 / N;
				if (current_progress != progress) {
					progress = current_progress;
					std::cout << "Progress: " << progress * 10 << "%" << std::endl;
				}
			}
			vertex_count = vertices.size();
			vertex_buffer.buffer_data(vertices.data(), vertex_count);
		}

		gl_error_guard(glCreateVertexArrays(1, &vertex_array));
		glBindVertexArray(vertex_array);

		auto &p = shaders.skybox_preview_program;
		glUseProgram(p.program_id);
		p.Model = glm::identity<glm::mat4>();

		vertex_buffer.bind([&](gl::VertexArrayBuilder b, const SkyboxVertex *v) {
			b.enable_attribute(p.position, v->view_position);
		});
	}

	void draw_preview(const glm::mat4 &ProjectionView) override {
		std::tuple<
				gl::disable_depth_test,
				gl::disable_depth_write,
				gl::disable_cull_face,
				gl::enable_blend>
				guard;
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		const Shaders::SkyboxPreviewProgram &p = shaders.skybox_preview_program;
		glBindVertexArray(vertex_array);
		glUseProgram(p.program_id);
		p.ProjectionView = ProjectionView;
		texture.bind(p.luminance = gl::TextureUnit(0));
		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	}

	void process_task(unique_ptr<Task> task) override {
		Task::done(std::move(task));
		// This is where the magic happens
	}
};

unique_ptr<Skybox> Skybox::init(const Shaders &shaders) {
	return make_unique<_Skybox>(shaders);
}