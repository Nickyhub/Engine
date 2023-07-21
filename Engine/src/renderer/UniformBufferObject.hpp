#pragma once

#include "glm/glm.hpp"

struct UniformBufferObject {
	glm::mat4 s_Model;
	glm::mat4 s_View;
	glm::mat4 s_Proj;
};