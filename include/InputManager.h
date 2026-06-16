#pragma once
#include <GLFW/glfw3.h>

class InputManager
{
public:
    InputManager();
    ~InputManager();

    static InputManager& GetInstance()
    {
        return *m_Instance;
    }

    static void DestroyInstance()
    {
        if (m_Instance)
        {
            delete m_Instance;
            m_Instance = nullptr;
        }
    }

    static bool Initialize();
    void Update();

    bool GetKeyDown(int key);
    bool GetKey(int key);
    bool GetKeyUp(int key);

private:
    static InputManager* m_Instance;

    bool m_currentKeys[GLFW_KEY_LAST + 1] = { false };
    bool m_previousKeys[GLFW_KEY_LAST + 1] = { false };
};

