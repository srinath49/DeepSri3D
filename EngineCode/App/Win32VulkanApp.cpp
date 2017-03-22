#include "Win32VulkanApp.hpp"
#include "EngineCode/Window/GlfwWindow.hpp"
#include "EngineCode/Renderer/VulkanRenderer.hpp"


//---------------------------------------------------------------------------------------------------
Win32VulkanApp::Win32VulkanApp()
	: BaseApp()
{
	m_window	= new GlfwWindow();
	m_renderer	= new VulkanRenderer();
}

//---------------------------------------------------------------------------------------------------
Win32VulkanApp::~Win32VulkanApp()
{
	
}

//---------------------------------------------------------------------------------------------------
void Win32VulkanApp::Initialize()
{
	m_window->Initialize();
	m_renderer->Initialize();
}

//---------------------------------------------------------------------------------------------------
void Win32VulkanApp::MainLoop()
{
	while (s_isRunning)
	{
		m_window->Update();
		m_renderer->Update();
		m_renderer->Draw();
	}
}
