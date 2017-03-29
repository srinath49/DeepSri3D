#pragma once


#ifndef _BASE_RENDERER_H_
#define _BASE_RENDERER_H_

//---------------------------------------------------------------------------------------------------
class BaseWindow;
class BaseApp;

//---------------------------------------------------------------------------------------------------
class BaseRenderer
{
public:
	BaseRenderer(BaseApp* appHandle);
	virtual ~BaseRenderer();

	virtual void Initialize(BaseWindow* window);
	virtual void Update();
	virtual void Draw();
	virtual void OnWindowResize(int width, int height);

protected:
	bool		m_isInitialized;
	BaseApp*	m_appHandle;
};
#endif // !_BASE_RENDERER_H_
