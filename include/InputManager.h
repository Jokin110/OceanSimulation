#pragma once

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

    bool GetKey(int key);

private:
    static InputManager* m_Instance;
};

