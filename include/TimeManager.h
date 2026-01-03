#pragma once

#include <chrono>

class TimeManager
{
public:
    TimeManager();
	~TimeManager();

    static TimeManager& GetInstance()
    {
        return *m_Instance;
	}

    static bool Initialize();
	void Update();

    float GetTime() const { return m_Time; }
    float GetDeltaTime() const { return m_DeltaTime; }
	float GetTimeScale() const { return m_TimeScale; }
	void SetTimeScale(float timeScale) { m_TimeScale = timeScale; }

private:
    static TimeManager* m_Instance;

	std::chrono::high_resolution_clock::time_point m_CurrentTime;

    float m_Time = 0.0f;
    float m_DeltaTime = 0.016f;
    float m_TimeScale = 1.0f;
};

