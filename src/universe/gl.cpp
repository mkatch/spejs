#include "gl.h"

#include <stdexcept>

void load_gl_api() {
	if (!gladLoadGL(glfwGetProcAddress)) {
		throw std::runtime_error("Failed to initialize OpenGL API");
	}
}