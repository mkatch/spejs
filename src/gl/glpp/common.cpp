#include <unordered_map>

#include "common.h"

namespace gl {

const string &enum_string(GLenum value) {
#define gl_enum_string_entry(name) \
	{ name, #name }
	static const std::unordered_map<GLenum, string> names = {
			gl_enum_string_entry(GL_NO_ERROR),
			gl_enum_string_entry(GL_INVALID_ENUM),
			gl_enum_string_entry(GL_INVALID_VALUE),
			gl_enum_string_entry(GL_INVALID_OPERATION),
			gl_enum_string_entry(GL_OUT_OF_MEMORY),
			gl_enum_string_entry(GL_FRAMEBUFFER_COMPLETE),
			gl_enum_string_entry(GL_FRAMEBUFFER_UNDEFINED),
			gl_enum_string_entry(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT),
			gl_enum_string_entry(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT),
			gl_enum_string_entry(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER),
			gl_enum_string_entry(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER),
			gl_enum_string_entry(GL_FRAMEBUFFER_UNSUPPORTED),
			gl_enum_string_entry(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE),
			gl_enum_string_entry(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS),
	};
#undef gl_enum_string_entry
	return names.at(value);
}

exception::exception(string const &message)
		: runtime_error(message) {}

exception::exception(string const &message, GLenum error_code)
		: runtime_error(message + " " + paren(enum_string(error_code))),
			_error_code(error_code) {}

}  // namespace gl