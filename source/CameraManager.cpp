#include "CameraManager.h"
#include "TimeManager.h"
#include "D3D11Application.h"
#include "InputManager.h"
#include "GLFW/glfw3.h"

#include "imgui.h"

CameraManager* CameraManager::m_Instance = nullptr;

CameraManager::CameraManager()
{
    m_ViewMatrix = XMMATRIX();
	m_ProjectionMatrix = XMMATRIX();
}

CameraManager::~CameraManager()
{
	if (m_Instance)
	{
		delete m_Instance;
		m_Instance = nullptr;
	}
}

bool CameraManager::Initialize()
{
	if (!m_Instance)
	{
		m_Instance = new CameraManager;
		return true;
	}

	return false;
}

void CameraManager::Start()
{

}

void CameraManager::Update()
{
    XMFLOAT3 cameraForward = XMFLOAT3(m_CameraFocusPoint.x - m_CameraPosition.x, 0, m_CameraFocusPoint.z - m_CameraPosition.z);
    float length = sqrtf(cameraForward.x * cameraForward.x + cameraForward.y * cameraForward.y + cameraForward.z * cameraForward.z);
    cameraForward = XMFLOAT3(cameraForward.x / length, 0, cameraForward.z / length);
    XMFLOAT3 cameraRight = XMFLOAT3(cameraForward.z, 0, -cameraForward.x);

    if (InputManager::GetInstance().GetKey(GLFW_KEY_W))
    {
        m_CameraPosition.x += m_Speed * TimeManager::GetInstance().GetUnscaledDeltaTime() * cameraForward.x;
        m_CameraPosition.z += m_Speed * TimeManager::GetInstance().GetUnscaledDeltaTime() * cameraForward.z;
    }
    if (InputManager::GetInstance().GetKey(GLFW_KEY_S))
    {
        m_CameraPosition.x -= m_Speed * TimeManager::GetInstance().GetUnscaledDeltaTime() * cameraForward.x;
        m_CameraPosition.z -= m_Speed * TimeManager::GetInstance().GetUnscaledDeltaTime() * cameraForward.z;
    }
    if (InputManager::GetInstance().GetKey(GLFW_KEY_A))
    {
        m_CameraPosition.x -= m_Speed * TimeManager::GetInstance().GetUnscaledDeltaTime() * cameraRight.x;
        m_CameraPosition.z -= m_Speed * TimeManager::GetInstance().GetUnscaledDeltaTime() * cameraRight.z;
    }
    if (InputManager::GetInstance().GetKey(GLFW_KEY_D))
    {
        m_CameraPosition.x += m_Speed * TimeManager::GetInstance().GetUnscaledDeltaTime() * cameraRight.x;
        m_CameraPosition.z += m_Speed * TimeManager::GetInstance().GetUnscaledDeltaTime() * cameraRight.z;
    }
    if (InputManager::GetInstance().GetKey(GLFW_KEY_Q))
    {
        m_CameraPosition.y += m_Speed * TimeManager::GetInstance().GetUnscaledDeltaTime();
	}
    if (InputManager::GetInstance().GetKey(GLFW_KEY_E))
    {
        m_CameraPosition.y -= m_Speed * TimeManager::GetInstance().GetUnscaledDeltaTime();
	}

    if (InputManager::GetInstance().GetKey(GLFW_KEY_P))
    {
        TimeManager::GetInstance().SetTimeScale(1.0f - TimeManager::GetInstance().GetTimeScale());
    }

    XMVECTOR eyePosition = XMLoadFloat3(&m_CameraPosition);
    XMVECTOR focusPosition = XMVectorSet(m_CameraFocusPoint.x, m_CameraFocusPoint.y, m_CameraFocusPoint.z, 1.0f);
    XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    m_ViewMatrix = XMMatrixLookAtLH(eyePosition, focusPosition, upDirection);
    m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(D3D11Application::GetInstance().GetWindowWidth()) / static_cast<float>(D3D11Application::GetInstance().GetWindowHeight()), 0.1f, 1000.0f);
}