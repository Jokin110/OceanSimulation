#include "D3D11Application.h"

int main(int argc, char* argv[])
{
    D3D11Application application{ "Ocean Simulation" };
    application.Run();

	std::cout << "Press Enter to exit...";
    std::cin.get();
	return 0;
}