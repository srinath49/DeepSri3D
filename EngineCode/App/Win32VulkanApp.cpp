#include "Win32VulkanApp.hpp"
#include "Main/PrecompiledDefinitions.hpp"
#include "EngineCode/Window/GlfwWindow.hpp"
#include "EngineCode/Renderer/VulkanRenderer.hpp"


//---------------------------------------------------------------------------------------------------
Win32VulkanApp::Win32VulkanApp()
	: BaseApp()
{
	m_window	= new GlfwWindow(this);
	m_renderer	= new VulkanRenderer(this);
}

//---------------------------------------------------------------------------------------------------
Win32VulkanApp::~Win32VulkanApp()
{
	
}

//---------------------------------------------------------------------------------------------------
void Win32VulkanApp::Initialize()
{
	m_window->Initialize();
	m_renderer->Initialize(m_window);
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

//---------------------------------------------------------------------------------------------------
void Win32VulkanApp::NotifyWindowResize(int width, int height)
{
	if (m_renderer)
	{
		m_renderer->OnWindowResize(width, height);
	}
}
