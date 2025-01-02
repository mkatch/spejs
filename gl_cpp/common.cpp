#include <format>
#include <unordered_map>

#include "common.h"

namespace gl {

const string &enum_string(GLenum value) {
#define _case(name) \
	{ name, #name }
#define _data_type_case(component_type) \
	_case(component_type),                \
			_case(component_type##_VEC2),     \
			_case(component_type##_VEC3),     \
			_case(component_type##_VEC4)

	static std::unordered_map<GLenum, string> names = {
		_case(GL_NO_ERROR),
		_case(GL_INVALID_ENUM),
		_case(GL_INVALID_VALUE),
		_case(GL_INVALID_OPERATION),
		_case(GL_OUT_OF_MEMORY),
		_case(GL_FRAMEBUFFER_COMPLETE),
		_case(GL_FRAMEBUFFER_UNDEFINED),
		_case(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT),
		_case(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT),
		_case(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER),
		_case(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER),
		_case(GL_FRAMEBUFFER_UNSUPPORTED),
		_case(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE),
		_case(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS),
		_data_type_case(GL_FLOAT),
		_data_type_case(GL_INT),
		_data_type_case(GL_UNSIGNED_INT),
		_data_type_case(GL_DOUBLE),
	};

#undef _data_type_case
#undef _case

	string &name = names[value];
	if (name.empty()) {
		name.append(std::format("GL_ENUM_0x{:x}", value));
	}
	return name;
}

exception::exception(string const &message)
		: runtime_error(message) {}

exception::exception(string const &message, GLenum error_code)
		: runtime_error(message + " " + paren(enum_string(error_code)))
		, _error_code(error_code) {}

}  // namespace gl