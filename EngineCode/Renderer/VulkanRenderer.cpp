#include "VulkanRenderer.hpp"
#include "Main/PrecompiledDefinitions.hpp"
#include "glfw3.h"
#include <iostream>
#include <set>
#include <algorithm>
#include "EngineCode/Window/GlfwWindow.hpp"
#include <fstream>
#include "EngineCode/App/Win32VulkanApp.hpp"
#include "VertexData.hpp"
#define GLM_FORCE_RADIANS
#include "ExtLibs/GLM/glm/glm.hpp"
#include "ExtLibs/GLM/glm/gtc/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "ExtLibs/stb/stb_image.h"
#include <chrono>


//---------------------------------------------------------------------------------------------------
const std::vector<Vertex> vertices = {
	{ { -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	{ { 0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
	{ { -0.5f, 0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } }
};

//---------------------------------------------------------------------------------------------------
const std::vector<uint16_t> indices = 
{
	0, 1, 2, 2, 3, 0
};

//---------------------------------------------------------------------------------------------------
struct UniformBufferObject 
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

//---------------------------------------------------------------------------------------------------
const int WIDTH		= 800;
const int HEIGHT	= 600;

//---------------------------------------------------------------------------------------------------
VulkanRenderer::VulkanRenderer(BaseApp* appHandle)
	: BaseRenderer(appHandle)
	, m_instance(VK_NULL_HANDLE)
	, m_enableValidationLayers(true)
	, m_validationCallback(VK_NULL_HANDLE)
/*	, m_surface(VK_NULL_HANDLE)*/
	, m_window(nullptr)
	, m_swapChain(VK_NULL_HANDLE)
{
	m_physicalDevices.reserve(5);
	m_logicalDevices.reserve(4);
}

//---------------------------------------------------------------------------------------------------
VulkanRenderer::~VulkanRenderer()
{
	Uninitialize();
}

//---------------------------------------------------------------------------------------------------
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::ValidationLayerCallback(VkDebugReportFlagsEXT flags,VkDebugReportObjectTypeEXT objType,uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData) 
{
	UNUSED(flags);
	UNUSED(objType);
	UNUSED(obj);
	UNUSED(location);
	UNUSED(code);
	UNUSED(layerPrefix);
	UNUSED(userData);
	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::Initialize(BaseWindow* window)
{
	m_window = window;
	CheckValidationLayers();
	CreateInstance();
	CreateSurface();
	SetupValidationLayerCallback();
	GatherPhysicalDevices();
	CreateLogicalDevice(m_physicalDevices[0]);
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout(m_logicalDevices[0]);
	CreateGraphicsPipeline();
	CreateFrameBuffers();
	CreateCommandPool();
	CreateTextureImage(m_logicalDevices[0]);
	CreateTextureImageView(m_logicalDevices[0], m_textureImage, m_textureImageView);
	CreateTextureSampler(m_logicalDevices[0]);
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffer();
	CreateDescriptorPool(m_logicalDevices[0]);
	CreateDescriptorSet(m_logicalDevices[0]);
	CreateCommandBuffers();
	CreateSemaphores();
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::Uninitialize()
{
	DestroySemaphores();
	DestroyCommandBuffers();
	DestroyDescriptorPool(m_logicalDevices[0]);
	DestroyUniformBuffer();
	DestroyIndexBuffer();
	DestroyVertexBuffer();
	DestroyTextureSampler(m_logicalDevices[0]);
	DestroyTextureImageView(m_logicalDevices[0], m_textureImageView);
	DestroyTextureImage(m_logicalDevices[0]);
	DestroyCommandPool();
	DestroyFrameBuffers();
	DestroyGraphicsPipeline();
	DestroyDescriptorSetLayout(m_logicalDevices[0]);
	DestroyRenderPass();
	DestroyImageViews();
	DestroySwapChain();
	DestroyLogicalDevices();
	DestroyPhysicalDevices();
	DestroyDebugReportCallbackEXT(m_instance, m_validationCallback, nullptr);
	DestroySurface();
	DestroyInstance();
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::RecreateSwapChain()
{
	vkDeviceWaitIdle(m_logicalDevices[0]);
	DestroyCommandBuffers();
	DestroyDescriptorPool(m_logicalDevices[0]);
	DestroyUniformBuffer();
	DestroyIndexBuffer();
	DestroyVertexBuffer();
	DestroyFrameBuffers();
	DestroyGraphicsPipeline();
	DestroyDescriptorSetLayout(m_logicalDevices[0]);
	DestroyRenderPass();
	DestroyTextureSampler(m_logicalDevices[0]);
	DestroyTextureImageView(m_logicalDevices[0], m_textureImageView);
	DestroyImageViews();

	CreateSwapChain();
	CreateImageViews();
	CreateTextureImageView(m_logicalDevices[0], m_textureImage, m_textureImageView);
	CreateTextureSampler(m_logicalDevices[0]);
	CreateRenderPass();
	CreateDescriptorSetLayout(m_logicalDevices[0]);
	CreateGraphicsPipeline();
	CreateFrameBuffers();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffer();
	CreateDescriptorPool(m_logicalDevices[0]);
	CreateDescriptorSet(m_logicalDevices[0]);
	CreateCommandBuffers();
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::Update()
{
	UpdateUniformBuffer(m_logicalDevices[0]);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::Draw()
{
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_logicalDevices[0], m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) 
	{
		RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	

	VkSubmitInfo submitInfo				= {};
	submitInfo.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[]		= { m_imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[]	= { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount		= 1;
	submitInfo.pWaitSemaphores			= waitSemaphores;
	submitInfo.pWaitDstStageMask		= waitStages;
	submitInfo.commandBufferCount		= 1;
	submitInfo.pCommandBuffers			= &m_commandBuffers[imageIndex];
	VkSemaphore signalSemaphores[]		= { m_renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount		= 1;
	submitInfo.pSignalSemaphores		= signalSemaphores;

	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo	= {};
	presentInfo.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount	= 1;
	presentInfo.pWaitSemaphores		= signalSemaphores;
	VkSwapchainKHR swapChains[]		= { m_swapChain };
	presentInfo.swapchainCount		= 1;
	presentInfo.pSwapchains			= swapChains;
	presentInfo.pImageIndices		= &imageIndex;
	presentInfo.pResults			= nullptr; // Optional

	result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
	{
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to present swap chain image!");
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::OnWindowResize(int width, int height)
{
	UNUSED(width);
	UNUSED(height);
	RecreateSwapChain();
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateInstance()
{
	if (m_enableValidationLayers && !CheckValidationLayers()) 
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "DeepSri Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	auto vulkanExtensions = GetRequiredExtensions();
	createInfo.enabledExtensionCount	= (uint32_t)vulkanExtensions.size();
	createInfo.ppEnabledExtensionNames	= vulkanExtensions.data();

	//createInfo.enabledExtensionCount = glfwExtensionCount;
	//createInfo.ppEnabledExtensionNames = glfwExtensions;
	
	if (m_enableValidationLayers) 
	{
		createInfo.enabledLayerCount	= (uint32_t)m_validationLayers.size();
		createInfo.ppEnabledLayerNames	= m_validationLayers.data();
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);

	if (result != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create instance!");
	}

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::cout << extensionCount << " extensions supported" << std::endl;

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "available extensions:" << std::endl;

	for (std::vector<VkExtensionProperties>::const_iterator extension = extensions.begin(); extension != extensions.end(); ++extension)
	{
		std::cout << "\t" << extension->extensionName << std::endl;
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyInstance()
{
	if (m_instance)
	{
		vkDestroyInstance(m_instance, nullptr);
		m_instance = VK_NULL_HANDLE;
	}
}

//---------------------------------------------------------------------------------------------------
bool VulkanRenderer::CheckValidationLayers()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : m_validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound) 
		{
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------------------------------------------------
std::vector<const char*> VulkanRenderer::GetRequiredExtensions()
{
	std::vector<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++) 
	{
		extensions.push_back(glfwExtensions[i]);
	}

	if (m_enableValidationLayers) 
	{
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::SetupValidationLayerCallback()
{
	if (m_enableValidationLayers)
	{
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = ValidationLayerCallback;
		//VkDebugReportCallbackEXT test;
		
		if (CreateDebugReportCallbackEXT(m_instance, &createInfo, nullptr, &m_validationCallback) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to set up debug callback!");
		}
	}
}

//---------------------------------------------------------------------------------------------------
VkResult VulkanRenderer::CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) 
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else 
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) 
	{
		func(instance, callback, pAllocator);
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::GatherPhysicalDevices()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

	if (deviceCount == 0) 
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	m_physicalDevices = std::vector < VkPhysicalDevice>(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, m_physicalDevices.data());
	int numValidDevices = 0;
	for (const auto& device : m_physicalDevices) 
	{
		if (IsPhysicalDeviceSuitable(device)) 
		{
			++numValidDevices;
		}
	}

	if (numValidDevices == 0) 
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	std::cout << "Number of suitable devices found: " << numValidDevices << std::endl;
}

//---------------------------------------------------------------------------------------------------
bool VulkanRenderer::IsPhysicalDeviceSuitable(VkPhysicalDevice deviceToCheck)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(deviceToCheck, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(deviceToCheck, &deviceFeatures);

	QueueFamilyIndices indices	= FindQueueFamilies(deviceToCheck);
	bool extensionsSupported	= CheckDeviceExtensioSupport(deviceToCheck);

	bool swapChainAdequate = false;
	if (extensionsSupported) 
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(deviceToCheck);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader && indices.IsComplete() && extensionsSupported && swapChainAdequate);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyPhysicalDevices()
{
	m_physicalDevices.clear();
}

//---------------------------------------------------------------------------------------------------
QueueFamilyIndices VulkanRenderer::FindQueueFamilies(const VkPhysicalDevice& deviceToFindQueues)
{
	QueueFamilyIndices indices;
	
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(deviceToFindQueues, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(deviceToFindQueues, &queueFamilyCount, queueFamilies.data());

	
	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(deviceToFindQueues, i, m_surface, &presentSupport);

		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}

		i++;
	}
	return indices;
}

//---------------------------------------------------------------------------------------------------
bool VulkanRenderer::CheckDeviceExtensioSupport(const VkPhysicalDevice& device)
{
	// Weird logic. Look into refactoring it later
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

	for (const auto& extension : availableExtensions) 
	{
		requiredExtensions.erase(extension.extensionName);
	}
	  
	return requiredExtensions.empty();
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateLogicalDevice(const VkPhysicalDevice& physicalDevice)
{
	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

	float queuePriority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.pQueueCreateInfos		= queueCreateInfos.data();
	createInfo.queueCreateInfoCount		= (uint32_t)queueCreateInfos.size();
	createInfo.pEnabledFeatures			= &deviceFeatures;
	createInfo.enabledExtensionCount	= (uint32_t)m_deviceExtensions.size();
	createInfo.ppEnabledExtensionNames	= m_deviceExtensions.data();

	if (m_enableValidationLayers) 
	{
		createInfo.enabledLayerCount	= (uint32_t)m_validationLayers.size();
		createInfo.ppEnabledLayerNames	= m_validationLayers.data();
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
	}

	VkDevice newLogicalDevice;
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &newLogicalDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
		return;
	}
	else
	{
		m_logicalDevices.push_back(newLogicalDevice);
	}
	
	vkGetDeviceQueue(newLogicalDevice, indices.graphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(newLogicalDevice, indices.presentFamily, 0, &m_presentQueue);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyLogicalDevices()
{
	for each (auto device in m_logicalDevices)
	{
		vkDestroyDevice(device, nullptr);
	}
	m_logicalDevices.clear();
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateSurface()
{
	if (m_window)
	{
		if (glfwCreateWindowSurface(m_instance, (GLFWwindow*)m_window->GetWindowHandle(), nullptr, &m_surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}

	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroySurface()
{
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

//---------------------------------------------------------------------------------------------------
SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport(const VkPhysicalDevice& device)
{
	SwapChainSupportDetails details;
	
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

	if (formatCount != 0) 
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) 
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

//---------------------------------------------------------------------------------------------------
VkSurfaceFormatKHR VulkanRenderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) 
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats) 
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return availableFormat;
		}
	}
	return availableFormats[0];
}

//---------------------------------------------------------------------------------------------------
VkPresentModeKHR VulkanRenderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
	for (const auto& availablePresentMode : availablePresentModes) 
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
		{
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) 
		{
			bestMode = availablePresentMode;
		}
	}
	return bestMode;
}

//---------------------------------------------------------------------------------------------------
VkExtent2D VulkanRenderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
	{
		return capabilities.currentExtent;
	}
	else 
	{
		VkExtent2D actualExtent = { WIDTH, HEIGHT };;
		if (m_appHandle)
		{
			int width, height;
			void* window = m_appHandle->GetWindowHandle();
			if (window)
			{
				GLFWwindow* glfwWindowHandle = static_cast<GLFWwindow*>(window);
				if (glfwWindowHandle)
				{
					glfwGetWindowSize(glfwWindowHandle, &width, &height);
					actualExtent = { (uint32_t)width, (uint32_t)height };
				}
			}
		}

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport	= QuerySwapChainSupport(m_physicalDevices[0]);
	VkSurfaceFormatKHR		surfaceFormat		= ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR		presentMode			= ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D				extent				= ChooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevices[0]);
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

	if (indices.graphicsFamily != indices.presentFamily) 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}
	createInfo.preTransform		= swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha	= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode		= presentMode;
	createInfo.clipped			= VK_TRUE;
	VkSwapchainKHR oldSwapChain = m_swapChain;
	createInfo.oldSwapchain		= oldSwapChain;

	VkSwapchainKHR newSwapChain;
	if (vkCreateSwapchainKHR(m_logicalDevices[0], &createInfo, nullptr, &newSwapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}
	DestroySwapChain();
	m_swapChain = newSwapChain;

	vkGetSwapchainImagesKHR(m_logicalDevices[0], m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_logicalDevices[0], m_swapChain, &imageCount, m_swapChainImages.data());
	m_swapChainImageFormat	= surfaceFormat.format;
	m_swapChainExtent		= extent;
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroySwapChain()
{
	vkDeviceWaitIdle(m_logicalDevices[0]);
	vkDestroySwapchainKHR(m_logicalDevices[0], m_swapChain, nullptr);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateImageViews()
{
	m_imageViews.resize(m_swapChainImages.size(), VkImageView());
	for (uint32_t i = 0; i < m_swapChainImages.size(); i++)
	{
		CreateImageView(m_logicalDevices[0], m_imageViews[i], m_swapChainImages[i], m_swapChainImageFormat);
	}
}

void VulkanRenderer::CreateImageView(const VkDevice& device, VkImageView& imageViewToCreate, const VkImage& imageToCreateViewFor, VkFormat imageFormat)
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = imageToCreateViewFor;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = imageFormat;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device, &createInfo, nullptr, &imageViewToCreate) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image views!");
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyImageViews()
{
	for each (auto view in m_imageViews)
	{
		DestroyImageView(m_logicalDevices[0], view);
	}
	m_imageViews.clear();
}

void VulkanRenderer::DestroyImageView(const VkDevice& device, VkImageView& imageViewToDestroy)
{
	vkDestroyImageView(device, imageViewToDestroy, nullptr);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateGraphicsPipeline()
{
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;

	auto vertShaderCode = ReadFile("EngineCode/Renderer/Shaders/DefaultShader.vert.spv");
	auto fragShaderCode = ReadFile("EngineCode/Renderer/Shaders/DefaultShader.frag.spv");

	CreateShaderModule(vertShaderCode, vertShaderModule);
	CreateShaderModule(fragShaderCode, fragShaderModule);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[]			= { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo	= {};
	auto bindingDescription									= Vertex::GetBindingDescription();
	auto attributeDescriptions								= Vertex::GetAttributeDescriptions();
	vertexInputInfo.sType									= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount			= 1;
	vertexInputInfo.pVertexBindingDescriptions				= &bindingDescription; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount			= attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions			= attributeDescriptions.data(); // Optional

	VkPipelineInputAssemblyStateCreateInfo inputAssembly	= {};
	inputAssembly.sType										= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology									= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable					= VK_FALSE;

	VkViewport viewport = {};
	viewport.x			= 0.0f;
	viewport.y			= 0.0f;
	viewport.width		= (float)m_swapChainExtent.width;
	viewport.height		= (float)m_swapChainExtent.height;
	viewport.minDepth	= 0.0f;
	viewport.maxDepth	= 1.0f;

	VkRect2D scissor	= {};
	scissor.offset		= { 0, 0 };
	scissor.extent		= m_swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState	= {};
	viewportState.sType								= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount						= 1;
	viewportState.pViewports						= &viewport;
	viewportState.scissorCount						= 1;
	viewportState.pScissors							= &scissor;


	VkPipelineRasterizationStateCreateInfo rasterizer	= {};
	rasterizer.sType									= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable							= VK_FALSE;
	rasterizer.rasterizerDiscardEnable					= VK_FALSE;
	rasterizer.polygonMode								= VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth								= 1.0f;
	rasterizer.cullMode									= VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace								= VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable							= VK_FALSE;
	rasterizer.depthBiasConstantFactor					= 0.0f; // Optional
	rasterizer.depthBiasClamp							= 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor						= 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling	= {};
	multisampling.sType									= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable					= VK_FALSE;
	multisampling.rasterizationSamples					= VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading						= 1.0f;		// Optional
	multisampling.pSampleMask							= nullptr;	// Optional
	multisampling.alphaToCoverageEnable					= VK_FALSE; // Optional
	multisampling.alphaToOneEnable						= VK_FALSE; // Optional


	VkPipelineColorBlendAttachmentState colorBlendAttachment	= {};
	colorBlendAttachment.colorWriteMask							= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable							= VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor					= VK_BLEND_FACTOR_SRC_ALPHA; // Optional
	colorBlendAttachment.dstColorBlendFactor					= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
	colorBlendAttachment.colorBlendOp							= VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor					= VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor					= VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp							= VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending	= {};
	colorBlending.sType									= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable							= VK_FALSE;
	colorBlending.logicOp								= VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount						= 1;
	colorBlending.pAttachments							= &colorBlendAttachment;
	colorBlending.blendConstants[0]						= 0.0f; // Optional
	colorBlending.blendConstants[1]						= 0.0f; // Optional
	colorBlending.blendConstants[2]						= 0.0f; // Optional
	colorBlending.blendConstants[3]						= 0.0f; // Optional

	VkDynamicState dynamicStates[] = 
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState	= {};
	dynamicState.sType								= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount					= 2;
	dynamicState.pDynamicStates						= dynamicStates;

	VkDescriptorSetLayout setLayouts[]				= { m_descriptorSetLayout };
	VkPipelineLayoutCreateInfo pipelineLayoutInfo	= {};
	pipelineLayoutInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount				= 1; 
	pipelineLayoutInfo.pSetLayouts					= setLayouts;
	pipelineLayoutInfo.pushConstantRangeCount		= 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges			= 0; // Optional

	if (vkCreatePipelineLayout(m_logicalDevices[0], &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo	= {};
	pipelineInfo.sType							= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount						= 2;
	pipelineInfo.pStages						= shaderStages;
	pipelineInfo.pVertexInputState				= &vertexInputInfo;
	pipelineInfo.pInputAssemblyState			= &inputAssembly;
	pipelineInfo.pViewportState					= &viewportState;
	pipelineInfo.pRasterizationState			= &rasterizer;
	pipelineInfo.pMultisampleState				= &multisampling;
	pipelineInfo.pDepthStencilState				= nullptr; // Optional
	pipelineInfo.pColorBlendState				= &colorBlending;
	pipelineInfo.pDynamicState					= nullptr; // Optional
	pipelineInfo.layout							= m_pipelineLayout;
	pipelineInfo.renderPass						= m_renderPass;
	pipelineInfo.subpass						= 0;
	pipelineInfo.basePipelineHandle				= VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex				= -1; // Optional: These values are only used if the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is also specified in the flags field of VkGraphicsPipelineCreateInfo

	if (vkCreateGraphicsPipelines(m_logicalDevices[0], VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	DestroyShaderModule(vertShaderModule);
	DestroyShaderModule(fragShaderModule);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyGraphicsPipeline()
{
	vkDestroyPipelineLayout(m_logicalDevices[0], m_pipelineLayout, nullptr);
	vkDestroyPipeline(m_logicalDevices[0], m_graphicsPipeline, nullptr);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateShaderModule(const std::vector<char>& code, VkShaderModule& shaderModuleToCreate)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();

	std::vector<uint32_t> codeAligned(code.size() / sizeof(uint32_t) + 1);
	memcpy(codeAligned.data(), code.data(), code.size());
	createInfo.pCode = codeAligned.data();

	if (vkCreateShaderModule(m_logicalDevices[0], &createInfo, nullptr, &shaderModuleToCreate) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyShaderModule(VkShaderModule& shaderModuleToDestroy)
{
	vkDestroyShaderModule(m_logicalDevices[0], shaderModuleToDestroy, nullptr);
}

//---------------------------------------------------------------------------------------------------
std::vector<char> VulkanRenderer::ReadFile(const std::string& fileName)
{
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);

		if (!file.is_open()) 
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment		= {};
	colorAttachment.format						= m_swapChainImageFormat;
	colorAttachment.samples						= VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp						= VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout				= VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout					= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef	= {};
	colorAttachmentRef.attachment				= 0;
	colorAttachmentRef.layout					= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass				= {};
	subpass.pipelineBindPoint					= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount				= 1;
	subpass.pColorAttachments					= &colorAttachmentRef;

	VkSubpassDependency dependency				= {};
	dependency.srcSubpass						= VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass						= 0;
	dependency.srcStageMask						= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask					= 0;
	dependency.dstStageMask						= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask					= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo		= {};
	renderPassInfo.sType						= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount				= 1;
	renderPassInfo.pAttachments					= &colorAttachment;
	renderPassInfo.subpassCount					= 1;
	renderPassInfo.pSubpasses					= &subpass;
	renderPassInfo.dependencyCount				= 1;
	renderPassInfo.pDependencies				= &dependency;

	if (vkCreateRenderPass(m_logicalDevices[0], &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyRenderPass()
{
	vkDestroyRenderPass(m_logicalDevices[0], m_renderPass, nullptr);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateFrameBuffers()
{
	m_swapChainFrameBuffers.resize(m_imageViews.size(), VkFramebuffer());

	for (size_t i = 0; i < m_imageViews.size(); i++)
	{
		VkImageView attachments[]				= { m_imageViews[i] };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType					= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass				= m_renderPass;
		framebufferInfo.attachmentCount			= 1;
		framebufferInfo.pAttachments			= attachments;
		framebufferInfo.width					= m_swapChainExtent.width;
		framebufferInfo.height					= m_swapChainExtent.height;
		framebufferInfo.layers					= 1;

		if (vkCreateFramebuffer(m_logicalDevices[0], &framebufferInfo, nullptr, &m_swapChainFrameBuffers[i]) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyFrameBuffers()
{
	for (auto frameBuffer : m_swapChainFrameBuffers)
	{
		vkDestroyFramebuffer(m_logicalDevices[0], frameBuffer, nullptr);
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevices[0]);
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = 0;

	if (vkCreateCommandPool(m_logicalDevices[0], &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyCommandPool()
{
	vkDestroyCommandPool(m_logicalDevices[0], m_commandPool, nullptr);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateCommandBuffers()
{
	DestroyCommandBuffers();
	m_commandBuffers.resize(m_swapChainFrameBuffers.size());

	VkCommandBufferAllocateInfo allocInfo	= {};
	allocInfo.sType							= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool					= m_commandPool;
	allocInfo.level							= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount			= (uint32_t)m_commandBuffers.size();

	if (vkAllocateCommandBuffers(m_logicalDevices[0], &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (size_t i = 0; i < m_commandBuffers.size(); i++) 
	{
		VkCommandBufferBeginInfo beginInfo	= {};
		beginInfo.sType						= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags						= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo			= nullptr; // Optional

		vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo	= {};
		renderPassInfo.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass				= m_renderPass;
		renderPassInfo.framebuffer				= m_swapChainFrameBuffers[i];
		renderPassInfo.renderArea.offset		= { 0, 0 };
		renderPassInfo.renderArea.extent		= m_swapChainExtent;

		VkClearValue clearColor					= { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount			= 1;
		renderPassInfo.pClearValues				= &clearColor;

		vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

		VkBuffer vertexBuffers[]	= { m_vertexBuffer };
		VkDeviceSize offsets[]		= { 0 };
		vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(m_commandBuffers[i], m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(m_commandBuffers[i], indices.size(), 1, 0, 0, 0);
		vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
		vkCmdEndRenderPass(m_commandBuffers[i]);

		if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyCommandBuffers()
{
	if (m_commandBuffers.size() > 0)
	{
		vkFreeCommandBuffers(m_logicalDevices[0], m_commandPool, (uint32_t)m_commandBuffers.size(), m_commandBuffers.data());
		m_commandBuffers.clear();
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType					= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(m_logicalDevices[0], &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS || vkCreateSemaphore(m_logicalDevices[0], &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create semaphores!");
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroySemaphores()
{
	vkDeviceWaitIdle(m_logicalDevices[0]);
	vkDestroySemaphore(m_logicalDevices[0], m_imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(m_logicalDevices[0], m_renderFinishedSemaphore, nullptr);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateVertexBuffer()
{
	VkDeviceSize	bufferSize = sizeof(vertices[0]) * vertices.size();
	VkBuffer		stagingBuffer;
	VkDeviceMemory	stagingBufferMemory;

	CreateStagingBuffer(m_logicalDevices[0], bufferSize, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_logicalDevices[0], stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_logicalDevices[0], stagingBufferMemory);

	CreateBuffer(m_logicalDevices[0], bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_vertexBuffer);
	AllocateBufferMemory(m_logicalDevices[0], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBufferMemory, m_vertexBuffer);
	CopyBuffer(m_logicalDevices[0], stagingBuffer, m_vertexBuffer, bufferSize);

	DestroyStagingBuffer(m_logicalDevices[0], stagingBuffer, stagingBufferMemory);
}



//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyVertexBuffer()
{
	FreeBufferMemory(m_logicalDevices[0], m_vertexBufferMemory);
	DestroyBuffer(m_logicalDevices[0], m_vertexBuffer);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateBuffer(const VkDevice& device, const VkDeviceSize& size, VkBufferUsageFlags usage, VkBuffer& buffer)
{
	VkBufferCreateInfo bufferInfo	= {};
	bufferInfo.sType				= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size					= size;
	bufferInfo.usage				= usage;
	bufferInfo.sharingMode			= VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyBuffer(const VkDevice& device, VkBuffer& bufferToFree)
{
	vkDestroyBuffer(device, bufferToFree, nullptr);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::AllocateBufferMemory(const VkDevice& device, VkMemoryPropertyFlags properties, VkDeviceMemory& bufferMemory, VkBuffer& bufferToAllocate)
{
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, bufferToAllocate, &memRequirements);

	VkMemoryAllocateInfo allocInfo	= {};
	allocInfo.sType					= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize		= memRequirements.size;
	allocInfo.memoryTypeIndex		= FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(device, bufferToAllocate, bufferMemory, 0);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::FreeBufferMemory(const VkDevice& device, VkDeviceMemory& bufferMemory)
{
	vkFreeMemory(device, bufferMemory, nullptr);
}

//---------------------------------------------------------------------------------------------------
uint32_t VulkanRenderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevices[0], &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateStagingBuffer(const VkDevice& device, VkDeviceSize size, VkBuffer& bufferToCreate, VkDeviceMemory& memoryToCreate)
{
	CreateBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bufferToCreate);
	AllocateBufferMemory(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryToCreate, bufferToCreate);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyStagingBuffer(const VkDevice& device, VkBuffer& bufferToDestroy, VkDeviceMemory& memoryToFree)
{
	FreeBufferMemory(device, memoryToFree);
	DestroyBuffer(device, bufferToDestroy);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CopyBuffer(const VkDevice& device, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo	= {};
	allocInfo.sType							= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level							= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool					= m_commandPool;
	allocInfo.commandBufferCount			= 1;

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, m_commandPool);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset	= 0; // Optional
	copyRegion.dstOffset	= 0; // Optional
	copyRegion.size			= size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(device, commandBuffer, m_commandPool, m_graphicsQueue);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateIndexBuffer()
{
	VkDeviceSize	bufferSize = sizeof(indices[0]) * indices.size();
	VkBuffer		stagingBuffer;
	VkDeviceMemory	stagingBufferMemory;
	CreateStagingBuffer(m_logicalDevices[0], bufferSize, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_logicalDevices[0], stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_logicalDevices[0], stagingBufferMemory);

	CreateBuffer(m_logicalDevices[0], bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_indexBuffer);
	AllocateBufferMemory(m_logicalDevices[0], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBufferMemory, m_indexBuffer);
	CopyBuffer(m_logicalDevices[0], stagingBuffer, m_indexBuffer, bufferSize);

	DestroyStagingBuffer(m_logicalDevices[0], stagingBuffer, stagingBufferMemory);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyIndexBuffer()
{
	FreeBufferMemory(m_logicalDevices[0], m_indexBufferMemory);
	DestroyBuffer(m_logicalDevices[0], m_indexBuffer);
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::CreateDescriptorSetLayout(const VkDevice& device)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding	= {};
	uboLayoutBinding.binding						= 0;
	uboLayoutBinding.descriptorType					= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount				= 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = bindings.size();
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroyDescriptorSetLayout(const VkDevice& device)
{
	vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
}

void VulkanRenderer::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	CreateStagingBuffer(m_logicalDevices[0], bufferSize, m_uniformStagingBuffer, m_uniformStagingBufferMemory);
	CreateBuffer(m_logicalDevices[0], bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, m_uniformBuffer);
	AllocateBufferMemory(m_logicalDevices[0], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_uniformBufferMemory, m_uniformBuffer);
}

void VulkanRenderer::DestroyUniformBuffer()
{
	DestroyStagingBuffer(m_logicalDevices[0], m_uniformStagingBuffer, m_uniformStagingBufferMemory);
	FreeBufferMemory(m_logicalDevices[0], m_uniformBufferMemory);
	DestroyBuffer(m_logicalDevices[0], m_uniformBuffer);
}

void VulkanRenderer::UpdateUniformBuffer(const VkDevice& device)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

	UniformBufferObject ubo = {};
	ubo.model = glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), m_swapChainExtent.width / (float)m_swapChainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(device, m_uniformStagingBufferMemory, 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, m_uniformStagingBufferMemory);

	CopyBuffer(device, m_uniformStagingBuffer, m_uniformBuffer, sizeof(ubo));

}

void VulkanRenderer::CreateDescriptorPool(const VkDevice& device)
{
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1;
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(m_logicalDevices[0], &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void VulkanRenderer::DestroyDescriptorPool(const VkDevice& device)
{
	vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
}

void VulkanRenderer::CreateDescriptorSet(const VkDevice& device)
{
	VkDescriptorSetLayout layouts[] = { m_descriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = m_textureImageView;
	imageInfo.sampler = m_textureSampler;

	if (vkAllocateDescriptorSets(device, &allocInfo, &m_descriptorSet) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to allocate descriptor set!");
	}

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = m_uniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(UniformBufferObject);

	std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = m_descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = m_descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void VulkanRenderer::CreateTextureImage(const VkDevice& device)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("EngineCode/Renderer/Textures/Texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) 
	{
		throw std::runtime_error("failed to load texture image!");
	}
	VkImage			stagingImage;
	VkDeviceMemory	stagingImageMemory;
	CreateImage(device, stagingImage, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_LAYOUT_PREINITIALIZED, texWidth, texHeight);
	AllocateImageMemory(device, stagingImageMemory, stagingImage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	BindImage(device, stagingImage, stagingImageMemory, 0);
	MapImage(device, stagingImage, stagingImageMemory, imageSize, texWidth, texHeight, pixels);
	UnmapImageMemory(device, stagingImageMemory);
	stbi_image_free(pixels);

	CreateImage(device, m_textureImage, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_PREINITIALIZED, texWidth, texHeight);
	AllocateImageMemory(device, m_textureImageMemory, m_textureImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	BindImage(device, m_textureImage, m_textureImageMemory, 0);

	TransitionImageLayout(device, m_commandPool, m_graphicsQueue, stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	TransitionImageLayout(device, m_commandPool, m_graphicsQueue, m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyImage(device, m_commandPool, m_graphicsQueue, VK_FORMAT_R8G8B8A8_UNORM, stagingImage, m_textureImage, texWidth, texHeight);
	TransitionImageLayout(device, m_commandPool, m_graphicsQueue, m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	FreeImageMemory(device, stagingImageMemory);
	DestroyImage(device, stagingImage);
}

void VulkanRenderer::DestroyTextureImage(const VkDevice& device)
{
	FreeImageMemory(device, m_textureImageMemory);
	DestroyImage(device, m_textureImage);
}

void VulkanRenderer::CreateImage(const VkDevice& device, VkImage& imageToCreate, VkFlags usage, VkFormat format, VkImageTiling tiling, VkImageLayout layout, uint32_t width, uint32_t height)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = layout;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0; // Optional

	if (vkCreateImage(device, &imageInfo, nullptr, &imageToCreate) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image!");
	}
}

void VulkanRenderer::DestroyImage(const VkDevice& device, VkImage& imageToDestroy)
{
	vkDestroyImage(device, imageToDestroy, nullptr);
}

void VulkanRenderer::AllocateImageMemory(const VkDevice& device, VkDeviceMemory& imageMemToAllocate, const VkImage& imageToAllocateMemFor, VkMemoryPropertyFlags memPropertyFlags)
{
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, imageToAllocateMemFor, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, memPropertyFlags);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemToAllocate) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory!");
	}
}

void VulkanRenderer::FreeImageMemory(const VkDevice& device, VkDeviceMemory& imageMemToFree)
{
	vkFreeMemory(device, imageMemToFree, nullptr);
}

void VulkanRenderer::BindImage(const VkDevice& device, const VkImage& imageToBind, const VkDeviceMemory& memoryToBind, uint32_t imageOffset)
{
	vkBindImageMemory(device, imageToBind, memoryToBind, imageOffset);
}

void VulkanRenderer::MapImage(const VkDevice& device, const VkImage& imageToMap, const VkDeviceMemory& memoryToMap, uint32_t imageSize, uint32_t width, uint32_t height, void* pixels)
{
	void* data;
	vkMapMemory(device, memoryToMap, 0, imageSize, 0, &data);

	VkImageSubresource subresource = {};
	subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource.mipLevel = 0;
	subresource.arrayLayer = 0;

	VkSubresourceLayout stagingImageLayout;
	vkGetImageSubresourceLayout(device, imageToMap, &subresource, &stagingImageLayout);

	if (stagingImageLayout.rowPitch == width * 4)
	{
		memcpy(data, pixels, (size_t)imageSize);
	}
	else
	{
		uint8_t* dataBytes = reinterpret_cast<uint8_t*>(data);
		stbi_uc* imagePixels = (stbi_uc*)pixels;
		for (int y = 0; y < height; y++)
		{
			memcpy
			(
				&dataBytes[y * stagingImageLayout.rowPitch],
				&imagePixels[y * width * 4],
				width * 4
			);
		}
	}
}

void VulkanRenderer::UnmapImageMemory(const VkDevice& device, const VkDeviceMemory& memoryToUnmap)
{
	vkUnmapMemory(device, memoryToUnmap);
}

VkCommandBuffer VulkanRenderer::BeginSingleTimeCommands(const VkDevice& device, const VkCommandPool& commandPool)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VulkanRenderer::EndSingleTimeCommands(const VkDevice& device, const VkCommandBuffer& commandBuffer, const VkCommandPool& commandPool, const VkQueue& queueToSubmit)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queueToSubmit, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queueToSubmit);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void VulkanRenderer::TransitionImageLayout(const VkDevice& device, const VkCommandPool& commandPool, const VkQueue& queueToSubmit, const VkImage& image, const VkFormat& format, const VkImageLayout& oldLayout, const VkImageLayout& newLayout)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) 
	{
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
	{
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	else 
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	EndSingleTimeCommands(device, commandBuffer, commandPool, queueToSubmit);
}

void VulkanRenderer::CopyImage(const VkDevice& device, const VkCommandPool& commandPool, const VkQueue& queueToSubmit, const VkFormat& format, const VkImage& srcImage, VkImage& dstImage, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);

	VkImageSubresourceLayers subResource = {};
	subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResource.baseArrayLayer = 0;
	subResource.mipLevel = 0;
	subResource.layerCount = 1;

	VkImageCopy region = {};
	region.srcSubresource = subResource;
	region.dstSubresource = subResource;
	region.srcOffset = { 0, 0, 0 };
	region.dstOffset = { 0, 0, 0 };
	region.extent.width = width;
	region.extent.height = height;
	region.extent.depth = 1;

	vkCmdCopyImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	EndSingleTimeCommands(device, commandBuffer, commandPool, queueToSubmit);
}

void VulkanRenderer::CreateTextureImageView(const VkDevice& device, const VkImage& imageToCreateViewFor, VkImageView& imageViewToCreate)
{
	CreateImageView(device, imageViewToCreate, imageToCreateViewFor, VK_FORMAT_R8G8B8A8_UNORM);
}

void VulkanRenderer::DestroyTextureImageView(const VkDevice& device, VkImageView& imageViewToDestroy)
{
	DestroyImageView(device, imageViewToDestroy);
}

void VulkanRenderer::CreateSampler(const VkDevice& device, VkSampler& samplerToCreate)
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &samplerToCreate) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void VulkanRenderer::DestroySampler(const VkDevice& device, VkSampler& samplerToDestroy)
{
	vkDestroySampler(device, samplerToDestroy, nullptr);
}

void VulkanRenderer::CreateTextureSampler(const VkDevice& device)
{
	CreateSampler(device, m_textureSampler);
}

void VulkanRenderer::DestroyTextureSampler(const VkDevice& device)
{
	DestroySampler(device, m_textureSampler);
}
