#pragma once
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <glm/glm.hpp>


struct UniformBufferObject {
	glm::mat4 s_Model;
	glm::mat4 s_View;
	glm::mat4 s_Proj;
};