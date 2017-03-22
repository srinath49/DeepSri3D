#pragma once

#ifndef _GLFW_WINDOW_H_
#define _GLFW_WINDOW_H_

//---------------------------------------------------------------------------------------------------
#include "BaseWindow.hpp"

//---------------------------------------------------------------------------------------------------
class GLFWwindow;

//---------------------------------------------------------------------------------------------------
class GlfwWindow : public BaseWindow
{
public:
	GlfwWindow();
	virtual ~GlfwWindow();

	void Initialize() override;
	void Uninitialize() override;
	void Update() override;

private:
	GLFWwindow*		m_glfwWindow;
};
#endif // !_GLFW_WINDOW_H_
