#pragma once

#include "common.h"

#include <iostream>

namespace gl {

struct disable_depth_test final {
	disable_depth_test() { glDisable(GL_DEPTH_TEST); }
	~disable_depth_test() { glEnable(GL_DEPTH_TEST); }
};

struct disable_depth_write final {
	disable_depth_write() { glDepthMask(GL_FALSE); }
	~disable_depth_write() { glDepthMask(GL_TRUE); }
};

struct disable_cull_face final {
	disable_cull_face() { glDisable(GL_CULL_FACE); }
	~disable_cull_face() { glEnable(GL_CULL_FACE); }
};

struct enable_blend final {
	enable_blend() { glEnable(GL_BLEND); }
	~enable_blend() { glDisable(GL_BLEND); }
};

}  // namespace gl