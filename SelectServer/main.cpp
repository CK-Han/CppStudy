#include <iostream>
#include <thread>

#include "Logic/LogicMain.h"


#include <string>
int main()
{
	std::cout << "server start" << std::endl;
	LogicLib::MainLogic main;
	main.Init();

	std::thread logicThread([&]() {
		main.Run();
	});
	
	std::cout << "press any key to exit...\n";
	getchar();

	main.Stop();
	logicThread.join();

	return 0;
}