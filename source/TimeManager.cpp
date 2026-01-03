#include "TimeManager.h"

TimeManager* TimeManager::m_Instance = nullptr;

TimeManager::TimeManager()
{
	m_CurrentTime = std::chrono::high_resolution_clock::now();

	m_Time = 0.0f;
	m_TimeScale = 1.0f;
}

TimeManager::~TimeManager()
{
	if (m_Instance)
	{
		delete m_Instance;
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
	m_DeltaTime = static_cast<float>(timeSpan.count() / 1000.0);

	m_Time += m_DeltaTime * m_TimeScale;
}