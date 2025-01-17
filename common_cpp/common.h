#pragma once

#include <cassert>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

using std::string;
using std::string_view;
using std::to_string;

inline string squote(const string_view &s) {
	return (std::ostringstream() << '\'' << s << '\'').str();
}
inline string paren(const string_view &s) {
	return (std::ostringstream() << '(' << s << ')').str();
}

using std::make_unique;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;