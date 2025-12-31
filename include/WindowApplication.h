#pragma once

#include <string>
#include <iostream>
#include <chrono>

struct GLFWwindow;

class WindowApplication
{
public:
    WindowApplication(const std::string& title);
    virtual ~WindowApplication();
    void Run();

protected:
	static void HandleResize(GLFWwindow* window, int32_t width, int32_t height);
	virtual void OnResize(int32_t width, int32_t height);

    virtual bool Initialize();
    virtual bool Load() = 0;
    virtual void Cleanup();
    virtual void Render() = 0;
    virtual void Update() = 0;

    [[nodiscard]] GLFWwindow* GetWindow() const;
    [[nodiscard]] int32_t GetWindowWidth() const;
    [[nodiscard]] int32_t GetWindowHeight() const;

    int32_t m_Width = 0;
	int32_t m_Height = 0;
    float m_Time = 0.0f;
    float m_DeltaTime = 0.016f;
    float m_TimeScale = 1.0f;

private:
	std::chrono::high_resolution_clock::time_point m_CurrentTime;
    GLFWwindow* m_Window = nullptr;
    std::string m_Title;
};