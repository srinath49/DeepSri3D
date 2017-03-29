#pragma once

#ifndef _BASE_APP_H_
#define _BASE_APP_H_

class BaseWindow;
class BaseRenderer;

//---------------------------------------------------------------------------------------------------
class BaseApp
{
public:
	BaseApp();
	virtual ~BaseApp();

	void Run();
	virtual void			NotifyWindowResize(int width, int height);

protected:
	virtual	void			Initialize();
	virtual	void			Uninitialize();
	virtual	void			MainLoop() = 0;

public:
	void*					GetWindowHandle();
protected:
	bool					m_isInitialized;
	BaseWindow*				m_window;
	BaseRenderer*			m_renderer;

public:
	static bool				s_isRunning;
};
#endif // !_BASE_APP_H_
