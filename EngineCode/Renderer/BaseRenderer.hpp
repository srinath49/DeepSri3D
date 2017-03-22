#pragma once


#ifndef _BASE_RENDERER_H_
#define _BASE_RENDERER_H_

//---------------------------------------------------------------------------------------------------
class BaseRenderer
{
public:
	BaseRenderer();
	virtual ~BaseRenderer();

	virtual void Initialize();
	virtual void Update();
	virtual void Draw();

protected:
	bool		m_isInitialized;
};
#endif // !_BASE_RENDERER_H_
