#include "BaseWindow.hpp"
#include "Main/PrecompiledDefinitions.hpp"

//---------------------------------------------------------------------------------------------------
BaseApp* BaseWindow::s_appHandle = nullptr;

//---------------------------------------------------------------------------------------------------
BaseWindow::BaseWindow(BaseApp* appHandle)
	: m_isInitialized(false)
{
	s_appHandle = appHandle;
}

//---------------------------------------------------------------------------------------------------
BaseWindow::~BaseWindow()
{
	s_appHandle = nullptr;
}

//---------------------------------------------------------------------------------------------------
void BaseWindow::Initialize()
{
	
}

//---------------------------------------------------------------------------------------------------
void BaseWindow::Uninitialize()
{
	
}

//---------------------------------------------------------------------------------------------------
void BaseWindow::Update()
{

}

//---------------------------------------------------------------------------------------------------
void* BaseWindow::GetWindowHandle()
{
	return nullptr;
}
