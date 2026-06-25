#pragma once

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_dx11.h"

class UIManager
{
public:
    UIManager();
    ~UIManager();

    static UIManager& GetInstance()
    {
        return *m_Instance;
    }

    static void DestroyInstance()
    {
        if (m_Instance)
        {
            m_Instance->CleanupImGui();

            delete m_Instance;
            m_Instance = nullptr;
        }
    }

    static bool Initialize();
    void Update();
    void Render();

private:
    static UIManager* m_Instance;

    bool InitializeImGui();
    void CleanupImGui();

    bool m_GUIActive = true;
};

