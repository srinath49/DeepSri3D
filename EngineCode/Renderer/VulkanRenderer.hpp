#pragma once

#ifndef _VULKAN_RENDERER_H_
#define _VULKAN_RENDERER_H_

//---------------------------------------------------------------------------------------------------
#include "BaseRenderer.hpp"
#include "vulkan\vulkan.h"
#include <vector>

//---------------------------------------------------------------------------------------------------
struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	bool IsComplete()
	{
		return graphicsFamily > -1;
	}
};

//---------------------------------------------------------------------------------------------------
class VulkanRenderer : public BaseRenderer
{
public:
	VulkanRenderer();
	virtual ~VulkanRenderer();

	void Initialize()	override;
	void Update()		override;
	void Draw()			override;

private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationLayerCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);

private:
	void						Uninitialize();
	void						CreateInstance();
	void						DestroyInstance();
	bool						CheckValidationLayers();
	std::vector<const char*>	GetRequiredExtensions();
	void						SetupValidationLayerCallback();
	VkResult					CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
	void						DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
	void						CreateCommandBuffers();
	void						GatherPhysicalDevices();
	bool						IsPhysicalDeviceSuitable(VkPhysicalDevice deviceToCheck);
	QueueFamilyIndices			FindQueueFamilies(VkPhysicalDevice device);

private:
	VkInstance						m_instance;
	bool							m_enableValidationLayers;
	const std::vector<const char*>	m_validationLayers = {/*(const char*)*/"VK_LAYER_LUNARG_standard_validation" };
	VkDebugReportCallbackEXT		m_validationCallback;
	std::vector<VkPhysicalDevice>	m_physicalDevices;
};
#endif // !_VULKAN_RENDERER_H_
