#include "InputManager.h"
#include "D3D11Application.h"

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

void InputManager::Update()
{
	GLFWwindow* window = D3D11Application::GetInstance().GetWindow();

	for (int i = 0; i <= GLFW_KEY_LAST; i++)
	{
		m_previousKeys[i] = m_currentKeys[i];

		m_currentKeys[i] = glfwGetKey(window, i) == GLFW_PRESS;
	}
}

bool InputManager::GetKeyDown(int key)
{
	if (key < 0 || key > GLFW_KEY_LAST) return false;
	return m_currentKeys[key] && !m_previousKeys[key];
}

bool InputManager::GetKey(int key)
{
	return glfwGetKey(D3D11Application::GetInstance().GetWindow(), key) == GLFW_PRESS;
}

bool InputManager::GetKeyUp(int key)
{
	if (key < 0 || key > GLFW_KEY_LAST) return false;
	return !m_currentKeys[key] && m_previousKeys[key];
}