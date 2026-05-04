#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct CameraSettings
{
    XMFLOAT3 m_Position;
    XMFLOAT3 m_FocusPoint;
    float m_Speed;
};

class CameraManager
{
public:
    CameraManager();
    ~CameraManager();

    static CameraManager& GetInstance()
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
    void Start();
    void Update();

    XMMATRIX GetViewMatrix() { return m_ViewMatrix; }
    XMMATRIX GetProjectionMatrix() { return m_ProjectionMatrix; }
    XMFLOAT3 GetCameraPosition() { return m_CameraPosition; }

private:
    static CameraManager* m_Instance;

    CameraSettings m_CameraSettings;

    float m_Speed = 5.0f;

    XMFLOAT3 m_CameraPosition = XMFLOAT3(0.0f, 50.0f, -150.0f);
    XMFLOAT3 m_CameraFocusPoint = XMFLOAT3(0.0f, 0.0f, 0.0f);

    XMMATRIX m_ViewMatrix;
    XMMATRIX m_ProjectionMatrix;
};

