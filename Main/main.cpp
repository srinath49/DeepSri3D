//---------------------------------------------------------------------------------------------------
#include "Main/PrecompiledDefinitions.hpp"
#include "EngineCode/App/Win32VulkanApp.hpp"
#include <stdexcept>
#include <iostream>


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
	std::cout << "Press enter to exit" << std::endl;
	getchar();
	return EXIT_SUCCESS;
}