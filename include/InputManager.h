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

    static bool Initialize();

    bool GetKey(int key);

private:
    static InputManager* m_Instance;
};

