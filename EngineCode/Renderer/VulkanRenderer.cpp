#include "VulkanRenderer.hpp"
#include "Main/PrecompiledDefinitions.hpp"
#include "glfw3.h"
#include <iostream>
#include <set>
#include <algorithm>
#include "EngineCOde/Window/GlfwWindow.hpp"

//---------------------------------------------------------------------------------------------------
const int WIDTH = 800;
const int HEIGHT = 600;

//---------------------------------------------------------------------------------------------------
VulkanRenderer::VulkanRenderer()
	: BaseRenderer()
	, m_instance(VK_NULL_HANDLE)
	, m_enableValidationLayers(true)
	, m_validationCallback(VK_NULL_HANDLE)
/*	, m_surface(VK_NULL_HANDLE)*/
	, m_window(nullptr)
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
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::Uninitialize()
{
	DestroyDebugReportCallbackEXT(m_instance, m_validationCallback, nullptr);
	DestroySwapChain();
	DestroyLogicalDevice();
	DestroySurface();
	DestroyInstance();
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::Update()
{

}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::Draw()
{

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
void VulkanRenderer::CreateCommandBuffers()
{

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
void VulkanRenderer::DestroyLogicalDevice()
{
	for each (auto device in m_logicalDevices)
	{
		vkDestroyDevice(device, nullptr);
	}
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
		VkExtent2D actualExtent = { WIDTH, HEIGHT };

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
	createInfo.oldSwapchain		= VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_logicalDevices[0], &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}
	vkGetSwapchainImagesKHR(m_logicalDevices[0], m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_logicalDevices[0], m_swapChain, &imageCount, m_swapChainImages.data());
	m_swapChainImageFormat	= surfaceFormat.format;
	m_swapChainExtent		= extent;
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::DestroySwapChain()
{
	vkDestroySwapchainKHR(m_logicalDevices[0], m_swapChain, nullptr);
}
