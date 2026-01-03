#include "WindowApplication.h"
#include <GLFW/glfw3.h>

#include <iostream>
#include <ratio>

WindowApplication::WindowApplication(const std::string& title)
{
    m_Title = title;
}

WindowApplication::~WindowApplication()
{
    Cleanup();
}

void WindowApplication::Run()
{
    if (!Initialize())
    {
        return;
    }

    if (!Load())
    {
        return;
	}

    while (!glfwWindowShouldClose(m_Window))
    {
        Update();
        Render();
    }
}

void WindowApplication::Cleanup()
{
    if (m_Window != nullptr)
    {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
    glfwTerminate();
}

bool WindowApplication::Initialize()
{
    if (!glfwInit())
    {
        std::cerr << "GLFW: Unable to initialize\n";
        return false;
    }

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    m_Width = static_cast<int32_t>(videoMode->width * 0.9f);
    m_Height = static_cast<int32_t>(videoMode->height * 0.9f);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.data(), nullptr, nullptr);

    if (m_Window == nullptr)
    {
        std::cerr << "GLFW: Unable to create window\n";
        Cleanup();
        return false;
    }

    const int32_t windowLeft = videoMode->width / 2 - m_Width / 2;
    const int32_t windowTop = videoMode->height / 2 - m_Height / 2;
    glfwSetWindowPos(m_Window, windowLeft, windowTop);

	glfwSetWindowUserPointer(m_Window, this);
	glfwSetFramebufferSizeCallback(m_Window, HandleResize);

    return true;
}

void WindowApplication::OnResize(int32_t width, int32_t height)
{
    m_Width = width;
    m_Height = height;
}

void WindowApplication::HandleResize(GLFWwindow* window, int32_t width, int32_t height)
{
    WindowApplication* application = static_cast<WindowApplication*>(glfwGetWindowUserPointer(window));
	application->OnResize(width, height);
}

GLFWwindow* WindowApplication::GetWindow() const
{
    return m_Window;
}

int32_t WindowApplication::GetWindowWidth() const
{
    return m_Width;
}

int32_t WindowApplication::GetWindowHeight() const
{
    return m_Height;
}

void WindowApplication::Update()
{
    glfwPollEvents();
}