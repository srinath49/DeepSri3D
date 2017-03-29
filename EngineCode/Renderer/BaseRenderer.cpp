#include "BaseRenderer.hpp"
#include "Main/PrecompiledDefinitions.hpp"
#include "EngineCode/Window/BaseWindow.hpp"

//---------------------------------------------------------------------------------------------------
BaseRenderer::BaseRenderer(BaseApp* appHandle)
	: m_isInitialized(false)
	, m_appHandle(appHandle)
{

}

//---------------------------------------------------------------------------------------------------
BaseRenderer::~BaseRenderer()
{
	m_appHandle = nullptr;
}

//---------------------------------------------------------------------------------------------------
void BaseRenderer::Initialize(BaseWindow* window)
{
	UNUSED(window);
}

//---------------------------------------------------------------------------------------------------
void BaseRenderer::Update()
{

}

//---------------------------------------------------------------------------------------------------
void BaseRenderer::Draw()
{
	
}

//---------------------------------------------------------------------------------------------------
void BaseRenderer::OnWindowResize(int width, int height)
{
	UNUSED(width);
	UNUSED(height);
}
