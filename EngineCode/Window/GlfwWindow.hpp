#pragma once

#ifndef _GLFW_WINDOW_H_
#define _GLFW_WINDOW_H_

//---------------------------------------------------------------------------------------------------
#include "BaseWindow.hpp"

//---------------------------------------------------------------------------------------------------
struct GLFWwindow;

//---------------------------------------------------------------------------------------------------
class GlfwWindow : public BaseWindow
{
public:
	GlfwWindow(BaseApp* appHandle);
	virtual ~GlfwWindow();

	void		Initialize() override;
	void		Uninitialize() override;
	void		Update() override;
	void*		GetWindowHandle() override;

	static void	OnWindowResize(GLFWwindow* window, int width, int height);
private:
	GLFWwindow*		m_glfwWindow;
};
#endif // !_GLFW_WINDOW_H_
