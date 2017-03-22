#include "GlfwWindow.hpp"
#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"
#include "EngineCode/App/BaseApp.hpp"

//---------------------------------------------------------------------------------------------------
GlfwWindow::GlfwWindow()
	: BaseWindow()
{
	glfwInit();
}

//---------------------------------------------------------------------------------------------------
GlfwWindow::~GlfwWindow()
{
	Uninitialize();
	glfwTerminate();
}

//---------------------------------------------------------------------------------------------------
void GlfwWindow::Initialize()
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_glfwWindow = glfwCreateWindow(1080, 900, "Window", nullptr, nullptr);

	if (m_glfwWindow)
	{
		m_isInitialized = true;
		BaseApp::s_isRunning = true;
	}
}

//---------------------------------------------------------------------------------------------------
void GlfwWindow::Uninitialize()
{
	if (m_isInitialized)
	{
		m_isInitialized = true;
		glfwDestroyWindow(m_glfwWindow);
	}
}

//---------------------------------------------------------------------------------------------------
void GlfwWindow::Update()
{
	BaseApp::s_isRunning = !glfwWindowShouldClose(m_glfwWindow);
	if (BaseApp::s_isRunning)
	{
		glfwPollEvents();
	}
}