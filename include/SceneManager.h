#pragma once

#include "OceanSurface.h"
#include "SkyBox.h"
#include <DirectXMath.h>

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

    PerObjectPixelShaderBufferData m_PixelShaderSettings;

    OceanSurface* m_Ocean[25] = { nullptr};
    //SkyBox m_SkyBox = SkyBox("SkyBox");
};

