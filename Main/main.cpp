//---------------------------------------------------------------------------------------------------
#include "EngineCode/App/Win32VulkanApp.hpp"
#include <stdexcept>
#include <iostream>

//---------------------------------------------------------------------------------------------------
#ifndef UNUSED
#define UNUSED (void)
#endif // !UNUSED

//---------------------------------------------------------------------------------------------------
int main()
{
	BaseApp* app = new Win32VulkanApp();

	try 
	{
		app->Run();
		delete app;
	}
	catch (const std::runtime_error& e) 
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}