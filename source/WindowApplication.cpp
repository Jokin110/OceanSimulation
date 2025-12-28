#include "WindowApplication.h"
#include <GLFW/glfw3.h>

#include <iostream>
#include <ratio>

WindowApplication::WindowApplication(const std::string& title)
{
    _title = title;
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

    while (!glfwWindowShouldClose(_window))
    {
        Update();
        Render();
    }
}

void WindowApplication::Cleanup()
{
    if (_window != nullptr)
    {
        glfwDestroyWindow(_window);
        _window = nullptr;
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
    _width = static_cast<int32_t>(videoMode->width * 0.9f);
    _height = static_cast<int32_t>(videoMode->height * 0.9f);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _window = glfwCreateWindow(_width, _height, _title.data(), nullptr, nullptr);

    if (_window == nullptr)
    {
        std::cerr << "GLFW: Unable to create window\n";
        Cleanup();
        return false;
    }

    const int32_t windowLeft = videoMode->width / 2 - _width / 2;
    const int32_t windowTop = videoMode->height / 2 - _height / 2;
    glfwSetWindowPos(_window, windowLeft, windowTop);

	glfwSetWindowUserPointer(_window, this);
	glfwSetFramebufferSizeCallback(_window, HandleResize);

	_currentTime = std::chrono::high_resolution_clock::now();

    return true;
}

void WindowApplication::OnResize(int32_t width, int32_t height)
{
    _width = width;
    _height = height;
}

void WindowApplication::HandleResize(GLFWwindow* window, int32_t width, int32_t height)
{
    WindowApplication* application = static_cast<WindowApplication*>(glfwGetWindowUserPointer(window));
	application->OnResize(width, height);
}

GLFWwindow* WindowApplication::GetWindow() const
{
    return _window;
}

int32_t WindowApplication::GetWindowWidth() const
{
    return _width;
}

int32_t WindowApplication::GetWindowHeight() const
{
    return _height;
}

void WindowApplication::Update()
{
	auto oldTime = _currentTime;
	_currentTime = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> timeSpan = (_currentTime - oldTime);
    _deltaTime = static_cast<float>(timeSpan.count() / 1000.0);
    glfwPollEvents();
}