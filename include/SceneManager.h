#pragma once

#include "OceanSurface.h"
#include "SkyBox.h"

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    static SceneManager& GetInstance()
    {
        return *m_Instance;
    }

    static bool Initialize();
    void Start();
    void Update();

private:
    static SceneManager* m_Instance;

    OceanSurface m_Ocean = OceanSurface("Ocean Surface" , L"assets/shaders/OceanSurfaceVS.hlsl", L"assets/shaders/PixelShader.hlsl", L"assets/shaders/OceanSurfaceHS.hlsl", L"assets/shaders/OceanSurfaceDS.hlsl", D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
    //SkyBox m_SkyBox = SkyBox("SkyBox");
};

