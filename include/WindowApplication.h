#pragma once

#include <string>
#include <iostream>

struct GLFWwindow;

class WindowApplication
{
public:
    WindowApplication(const std::string& title);
    virtual ~WindowApplication();
    void Run();

    GLFWwindow* GetWindow() const;
    int32_t GetWindowWidth() const;
    int32_t GetWindowHeight() const;

protected:
	static void HandleResize(GLFWwindow* window, int32_t width, int32_t height);
	virtual void OnResize(int32_t width, int32_t height);

    virtual bool Initialize();
    virtual bool Load() = 0;
    virtual void Cleanup();
    virtual void Render() = 0;
    virtual void Update() = 0;

    int32_t m_Width = 0;
	int32_t m_Height = 0;

private:
    GLFWwindow* m_Window = nullptr;
    std::string m_Title;
};