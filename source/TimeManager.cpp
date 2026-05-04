#include "TimeManager.h"
#include <imgui.h>

TimeManager* TimeManager::m_Instance = nullptr;

TimeManager::TimeManager()
{
	m_CurrentTime = std::chrono::high_resolution_clock::now();

	m_Time = 0.0f;
	m_UnscaledTime = 0.0f;
	m_TimeScale = 1.0f;
}

TimeManager::~TimeManager()
{
	if (m_Instance)
	{
		m_Instance = nullptr;
	}
}

bool TimeManager::Initialize()
{
	if (!m_Instance)
	{
		m_Instance = new TimeManager;
		return true;
	}

	return false;
}

void TimeManager::Update()
{
	auto oldTime = m_CurrentTime;
	m_CurrentTime = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> timeSpan = (m_CurrentTime - oldTime);

	m_UnscaledDeltaTime = static_cast<float>(timeSpan.count() / 1000.0);
	m_DeltaTime = m_UnscaledDeltaTime * m_TimeScale;

	m_Time += m_DeltaTime;
	m_UnscaledTime += m_UnscaledDeltaTime;

	ImGui::Begin("FPS");
	float deltaTime = m_UnscaledDeltaTime > 0.0f ? m_UnscaledDeltaTime : 0.00001f; // Avoid division by zero
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", m_UnscaledDeltaTime * 1000.0f, 1.0f / deltaTime);
	ImGui::End();
}