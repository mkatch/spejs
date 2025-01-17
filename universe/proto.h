#pragma once

#include <google/protobuf/repeated_field.h>

#include "common.h"
#include "math.h"

template <class T, class P>
T proto_cast(const P &p) {
	static_assert(sizeof(T) == 0, "Explicit specialization required");
}

template <>
inline glm::vec3 proto_cast<glm::vec3, google::protobuf::RepeatedField<float>>(const google::protobuf::RepeatedField<float> &proto) {
	return proto.size() == 3
			? glm::vec3(proto[0], proto[1], proto[2])
			: glm::vec3(NAN, NAN, NAN);
}
