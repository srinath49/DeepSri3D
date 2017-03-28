#include "BaseRenderer.hpp"
#include "Main/PrecompiledDefinitions.hpp"
#include "../Window/BaseWindow.hpp"

//---------------------------------------------------------------------------------------------------
BaseRenderer::BaseRenderer()
	: m_isInitialized(false)
{

}

//---------------------------------------------------------------------------------------------------
BaseRenderer::~BaseRenderer()
{

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