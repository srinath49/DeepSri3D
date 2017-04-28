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
	void									CreateImageView(const VkDevice& device, VkImageView& imageViewToCreate, const VkImage& imageToCreateViewFor, VkFormat imageFormat);
	void									DestroyImageViews();
	void									DestroyImageView(const VkDevice& device, VkImageView& imageViewToDestroy);
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
	void									CreateVertexBuffer();
	void									DestroyVertexBuffer();
	void									CreateBuffer(const VkDevice& device, const VkDeviceSize& size, VkBufferUsageFlags usage, VkBuffer& buffer);
	void									DestroyBuffer(const VkDevice& device, VkBuffer& bufferToFree);
	void									AllocateBufferMemory(const VkDevice& device, VkMemoryPropertyFlags properties, VkDeviceMemory& bufferMemory, VkBuffer& bufferToAllocate);
	void									FreeBufferMemory(const VkDevice& device, VkDeviceMemory& bufferMemory);
	uint32_t								FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void									CreateStagingBuffer(const VkDevice& device, VkDeviceSize size, VkBuffer& bufferToCreate, VkDeviceMemory& memoryToCreate);
	void									DestroyStagingBuffer(const VkDevice& device, VkBuffer& bufferToDestroy, VkDeviceMemory& memoryToFree);
	void									CopyBuffer(const VkDevice& device, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size);
	void									CreateIndexBuffer();
	void									DestroyIndexBuffer();
	void									CreateDescriptorSetLayout(const VkDevice& device);
	void									DestroyDescriptorSetLayout(const VkDevice& device);
	void									CreateUniformBuffer();
	void									DestroyUniformBuffer();
	void									UpdateUniformBuffer(const VkDevice& device);
	void									CreateDescriptorPool(const VkDevice& device);
	void									DestroyDescriptorPool(const VkDevice& device);
	void									CreateDescriptorSet(const VkDevice& device);
	void									CreateTextureImage(const VkDevice& device);
	void									DestroyTextureImage(const VkDevice& device);
	void									CreateImage(const VkDevice& device, VkImage& imageToCreate, VkFlags usage, VkFormat format, VkImageTiling tiling, VkImageLayout layout, uint32_t width, uint32_t height);
	void									DestroyImage(const VkDevice& device, VkImage& imageToDestroy);
	void									AllocateImageMemory(const VkDevice& device, VkDeviceMemory& imageMemToAllocate, const VkImage& imageToAllocateMemFor, VkMemoryPropertyFlags memPropertyFlags);
	void									FreeImageMemory(const VkDevice& device, VkDeviceMemory& imageMemToFree);
	void									BindImage(const VkDevice& device, const VkImage& imageToBind, const VkDeviceMemory& memoryToBind, uint32_t imageOffset);
	void									MapImage(const VkDevice& device, const VkImage& imageToMap, const VkDeviceMemory& memoryToMap, uint32_t imageSize, uint32_t width, uint32_t height, void* pixels);
	void									UnmapImageMemory(const VkDevice& device, const VkDeviceMemory& memoryToBind);
	VkCommandBuffer							BeginSingleTimeCommands(const VkDevice& device, const VkCommandPool& commandPool);
	void									EndSingleTimeCommands(const VkDevice& device, const VkCommandBuffer& commandBuffer, const VkCommandPool& commandPool, const VkQueue& queueToSubmit);
	void									TransitionImageLayout(const VkDevice& device, const VkCommandPool& commandPool, const VkQueue& queueToSubmit, const VkImage& image, const VkFormat& format, const VkImageLayout& oldLayout, const VkImageLayout& newLayout);
	void									CopyImage(const VkDevice& device, const VkCommandPool& commandPool, const VkQueue& queueToSubmit, const VkFormat& format, const VkImage& srcImage, VkImage& dstImage, uint32_t width, uint32_t height);
	void									CreateTextureImageView(const VkDevice& device, const VkImage& imageToCreateViewFor, VkImageView& imageViewToCreate);
	void									DestroyTextureImageView(const VkDevice& device, VkImageView& imageViewToDestroy);
	void									CreateSampler(const VkDevice& device, VkSampler& samplerToCreate);
	void									DestroySampler(const VkDevice& device, VkSampler& samplerToDestroy);
	void									CreateTextureSampler(const VkDevice& device);
	void									DestroyTextureSampler(const VkDevice& device);


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
	VkDescriptorSetLayout					m_descriptorSetLayout;
	VkPipelineLayout						m_pipelineLayout;
	VkRenderPass							m_renderPass;
	VkPipeline								m_graphicsPipeline;
	std::vector<VkFramebuffer>				m_swapChainFrameBuffers;
	VkCommandPool							m_commandPool;
	std::vector<VkCommandBuffer>			m_commandBuffers;
	VkSemaphore								m_imageAvailableSemaphore;
	VkSemaphore								m_renderFinishedSemaphore;
	VkBuffer								m_vertexBuffer;
	VkDeviceMemory							m_vertexBufferMemory;
	VkBuffer								m_indexBuffer;
	VkDeviceMemory							m_indexBufferMemory;
	VkBuffer								m_uniformBuffer;
	VkDeviceMemory							m_uniformBufferMemory;
	VkBuffer								m_uniformStagingBuffer;
	VkDeviceMemory							m_uniformStagingBufferMemory;
	VkDescriptorPool						m_descriptorPool;
	VkDescriptorSet							m_descriptorSet;
	VkImage									m_textureImage;
	VkDeviceMemory							m_textureImageMemory;
	VkImageView								m_textureImageView;
	VkSampler								m_textureSampler;

};
#endif // !_VULKAN_RENDERER_H_
