#include "BaseApp.hpp"
#include "Main/PrecompiledDefinitions.hpp"
#include "EngineCode/Renderer/BaseRenderer.hpp"
#include "EngineCode/Window/BaseWindow.hpp"


//---------------------------------------------------------------------------------------------------
bool BaseApp::s_isRunning = false;

//---------------------------------------------------------------------------------------------------
BaseApp::BaseApp()
	: m_isInitialized(false)
	, m_window(nullptr)
	, m_renderer(nullptr)
{

}

//---------------------------------------------------------------------------------------------------
BaseApp::~BaseApp()
{
	Uninitialize();
}

//---------------------------------------------------------------------------------------------------
void BaseApp::Run()
{
	Initialize();
	MainLoop();
}

//---------------------------------------------------------------------------------------------------
void BaseApp::Initialize()
{
	
}

//---------------------------------------------------------------------------------------------------
void BaseApp::Uninitialize()
{
	if (m_window)
	{
		delete m_window;
	}

	if (m_renderer)
	{
		delete m_renderer;
	}

	m_isInitialized = false;
}