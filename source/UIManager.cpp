#include "UIManager.h"
#include "D3D11Application.h"
#include "InputManager.h"

UIManager* UIManager::m_Instance = nullptr;

UIManager::UIManager()
{
	m_GUIActive = true;
}

UIManager::~UIManager()
{
	if (m_Instance)
	{
		m_Instance = nullptr;
	}
}

bool UIManager::Initialize()
{
	if (!m_Instance)
	{
		m_Instance = new UIManager;

		m_Instance->InitializeImGui();

		return true;
	}

	return false;
}

void UIManager::Update()
{
	if (InputManager::GetInstance().GetKeyDown(GLFW_KEY_T))
	{
		m_GUIActive = !m_GUIActive;
	}
}

void UIManager::Render()
{
	if (m_GUIActive)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		D3D11Application::GetInstance().UpdateUI();

		ImGui::Render();

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
}

bool UIManager::InitializeImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOther(D3D11Application::GetInstance().GetWindow(), true);
	ImGui_ImplDX11_Init(D3D11Application::GetInstance().GetDevice(), D3D11Application::GetInstance().GetDeviceContext());

	return true;
}

void UIManager::CleanupImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
