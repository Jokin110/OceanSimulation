#include "InputManager.h"
#include "D3D11Application.h"
#include <GLFW/glfw3.h>

InputManager* InputManager::m_Instance = nullptr;

InputManager::InputManager()
{

}

InputManager::~InputManager()
{
	if (m_Instance)
	{
		m_Instance = nullptr;
	}
}

bool InputManager::Initialize()
{
	if (!m_Instance)
	{
		m_Instance = new InputManager;
		return true;
	}

	return false;
}

bool InputManager::GetKey(int key)
{
	return glfwGetKey(D3D11Application::GetInstance().GetWindow(), key) == GLFW_PRESS;
}