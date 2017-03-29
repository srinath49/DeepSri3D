#pragma once

#ifndef _BASE_WINDOW_H_
#define _BASE_WINDOW_H_

//---------------------------------------------------------------------------------------------------
class BaseApp;

//---------------------------------------------------------------------------------------------------
class BaseWindow
{
public:
	BaseWindow(BaseApp* appHandle);
	virtual ~BaseWindow();

	virtual void	Initialize();
	virtual void	Uninitialize();
	virtual void	Update();
	virtual void*	GetWindowHandle();

protected:
	bool			m_isInitialized;
	static BaseApp*	s_appHandle;

};
#endif // !_BASE_WINDOW_H_
