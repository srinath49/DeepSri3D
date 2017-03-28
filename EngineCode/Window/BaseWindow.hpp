#pragma once

#ifndef _BASE_WINDOW_H_
#define _BASE_WINDOW_H_

//---------------------------------------------------------------------------------------------------
class BaseWindow
{
public:
	BaseWindow();
	virtual ~BaseWindow();

	virtual void	Initialize();
	virtual void	Uninitialize();
	virtual void	Update();
	virtual void*	GetWindowHandle();

protected:
	bool		m_isInitialized;

};
#endif // !_BASE_WINDOW_H_
