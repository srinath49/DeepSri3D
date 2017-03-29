#pragma once

#ifndef _APP_WIN32_VULKAN_H_
#define _APP_WIN32_VULKAN_H_

//---------------------------------------------------------------------------------------------------
#include "BaseApp.hpp"

//---------------------------------------------------------------------------------------------------
class Win32VulkanApp : public BaseApp
{
public:
	Win32VulkanApp();
	virtual ~Win32VulkanApp();

protected:
	virtual void Initialize();
	virtual void MainLoop();

public:
	void NotifyWindowResize(int width, int height) override;
};
#endif // !_APP_WIN32_VULKAN_H_
