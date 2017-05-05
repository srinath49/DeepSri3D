#pragma once

#ifndef _VERTEX_DATA_H_
#define _VERTEX_DATA_H_

//---------------------------------------------------------------------------------------------------
#include "ExtLibs/GLM/glm/glm.hpp"
#include <array>
#include "vulkan/vulkan.h"

//---------------------------------------------------------------------------------------------------
struct Vertex
{
	glm::vec3	pos;
	glm::vec3	color;
	glm::vec2	texCoords;

	static VkVertexInputBindingDescription GetBindingDescription() 
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding		= 0;
		bindingDescription.stride		= sizeof(Vertex);
		bindingDescription.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
		attributeDescriptions[0].binding	= 0;
		attributeDescriptions[0].location	= 0;
		attributeDescriptions[0].format		= VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset		= offsetof(Vertex, pos);
		attributeDescriptions[1].binding	= 0;
		attributeDescriptions[1].location	= 1;
		attributeDescriptions[1].format		= VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset		= offsetof(Vertex, color);
		attributeDescriptions[2].binding	= 0;
		attributeDescriptions[2].location	= 2;
		attributeDescriptions[2].format		= VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset		= offsetof(Vertex, texCoords);

		return attributeDescriptions;
	}
};
#endif // !_VERTEX_DATA_H_
