#include "D3D11Application.h"

int main(int argc, char* argv[])
{
    if (D3D11Application::InitializeInstance("Ocean Simulation"))
    {
        D3D11Application::GetInstance().Run();
    }

	//std::cout << "Press Enter to exit...";
    //std::cin.get();
	return 0;
}