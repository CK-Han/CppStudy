#include <iostream>
#include "Framework.h"

int main()
{
	Framework framework;
	framework.Initialize();

	while (framework.IsEnded() == false) {
		framework.Run();
	}
}
