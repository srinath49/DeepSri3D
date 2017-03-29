#pragma once

#ifndef _VULKAN_RENDERER_H_
#define _VULKAN_RENDERER_H_

//---------------------------------------------------------------------------------------------------
#include "BaseRenderer.hpp"
#include "vulkan\vulkan.h"
#include <vector>

//---------------------------------------------------------------------------------------------------
class BaseWindow;
class BaseApp;

//---------------------------------------------------------------------------------------------------
struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;
	bool IsComplete()
	{
		return graphicsFamily > -1 && presentFamily > -1;
	}
};

//---------------------------------------------------------------------------------------------------
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR		capabilities;
	std::vector<VkSurfaceFormatKHR>	formats;
	std::vector<VkPresentModeKHR>	presentModes;
};

//---------------------------------------------------------------------------------------------------
class VulkanRenderer : public BaseRenderer
{
public:
	VulkanRenderer(BaseApp* appHandle);
	virtual ~VulkanRenderer();

	void Initialize(BaseWindow* window) override;
	void Update()		override;
	void Draw()			override;

	void OnWindowResize(int width, int height) override;

private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL	ValidationLayerCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);

private:
	void									Uninitialize();
	void									CreateInstance();
	void									DestroyInstance();
	bool									CheckValidationLayers();
	std::vector<const char*>				GetRequiredExtensions();
	void									SetupValidationLayerCallback();
	VkResult								CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
	void									DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
	void									GatherPhysicalDevices();
	bool									IsPhysicalDeviceSuitable(VkPhysicalDevice deviceToCheck);
	void									DestroyPhysicalDevices();
	QueueFamilyIndices						FindQueueFamilies(const VkPhysicalDevice& device);
	bool									CheckDeviceExtensioSupport(const VkPhysicalDevice& device);
	void									CreateLogicalDevice(const VkPhysicalDevice& physicalDevice);
	void									DestroyLogicalDevices();
	void									CreateSurface();
	void									DestroySurface();
	SwapChainSupportDetails					QuerySwapChainSupport(const VkPhysicalDevice& device);
	VkSurfaceFormatKHR						ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR						ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D								ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void									CreateSwapChain();
	void									DestroySwapChain();
	void									CreateImageViews();
	void									DestroyImageViews();
	void									CreateGraphicsPipeline();
	void									DestroyGraphicsPipeline();
	void									CreateShaderModule(const std::vector<char>& code, VkShaderModule& shaderModuleToCreate);
	void									DestroyShaderModule(VkShaderModule& shaderModuleToDestroy);
	static std::vector<char>				ReadFile(const std::string& fileName);
	void									CreateRenderPass();
	void									DestroyRenderPass();
	void									CreateFrameBuffers();
	void									DestroyFrameBuffers();
	void									CreateCommandPool();
	void									DestroyCommandPool();
	void									CreateCommandBuffers();
	void									DestroyCommandBuffers();
	void									CreateSemaphores();
	void									DestroySemaphores();
	void									RecreateSwapChain();

private:
	VkInstance								m_instance;
	bool									m_enableValidationLayers;
	const std::vector<const char*>			m_validationLayers = {/*(const char*)*/"VK_LAYER_LUNARG_standard_validation" };
	const std::vector<const char*>			m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkDebugReportCallbackEXT				m_validationCallback;
	std::vector<VkPhysicalDevice>			m_physicalDevices;
	std::vector<VkDevice>					m_logicalDevices;
	VkSurfaceKHR							m_surface;
	BaseWindow*								m_window;
	VkQueue									m_graphicsQueue;
	VkQueue									m_presentQueue;
	VkSwapchainKHR							m_swapChain;
	std::vector<VkImage>					m_swapChainImages;
	VkFormat								m_swapChainImageFormat;
	VkExtent2D								m_swapChainExtent;
	std::vector<VkImageView>				m_imageViews;
	VkPipelineLayout						m_pipelineLayout;
	VkRenderPass							m_renderPass;
	VkPipeline								m_graphicsPipeline;
	std::vector<VkFramebuffer>				m_swapChainFrameBuffers;
	VkCommandPool							m_commandPool;
	std::vector<VkCommandBuffer>			m_commandBuffers;
	VkSemaphore								m_imageAvailableSemaphore;
	VkSemaphore								m_renderFinishedSemaphore;

};
#endif // !_VULKAN_RENDERER_H_
