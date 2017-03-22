#include "VulkanRenderer.hpp"
#include "glfw3.h"
#include <iostream>


//---------------------------------------------------------------------------------------------------
VulkanRenderer::VulkanRenderer()
	: BaseRenderer()
	, m_instance(VK_NULL_HANDLE)
	, m_enableValidationLayers(true)
{
	m_physicalDevices.reserve(5);
}

//---------------------------------------------------------------------------------------------------
VulkanRenderer::~VulkanRenderer()
{
	Uninitialize();
}

//---------------------------------------------------------------------------------------------------
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::ValidationLayerCallback(VkDebugReportFlagsEXT flags,VkDebugReportObjectTypeEXT objType,uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData) 
{

	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::Initialize()
{
	CheckValidationLayers();
	CreateInstance();
	SetupValidationLayerCallback();
	GatherPhysicalDevices();
}

//---------------------------------------------------------------------------------------------------
void VulkanRenderer::Uninitialize()
{
	DestroyDebugReportCallbackEXT(m_instance, m_validationCallback, nullptr);
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
	createInfo.enabledExtensionCount = vulkanExtensions.size();
	createInfo.ppEnabledExtensionNames = vulkanExtensions.data();

	//createInfo.enabledExtensionCount = glfwExtensionCount;
	//createInfo.ppEnabledExtensionNames = glfwExtensions;
	
	if (m_enableValidationLayers) 
	{
		createInfo.enabledLayerCount = m_validationLayers.size();
		createInfo.ppEnabledLayerNames = m_validationLayers.data();
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
		VkDebugReportCallbackEXT test;
		
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
}

//---------------------------------------------------------------------------------------------------
bool VulkanRenderer::IsPhysicalDeviceSuitable(VkPhysicalDevice deviceToCheck)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(deviceToCheck, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(deviceToCheck, &deviceFeatures);

	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
}
