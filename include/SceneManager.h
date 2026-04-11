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

	float GetFoamBias() const { return m_PixelShaderSettings.m_FoamBias; }
	float GetDecayFactor() const { return m_PixelShaderSettings.m_DecayFactor; }

private:
    static SceneManager* m_Instance;

    PerObjectPixelShaderBufferData m_PixelShaderSettings;

	static const int m_OceanSurfaceSideCount = 5; // Number of ocean surfaces along one side of the grid (total surfaces = oceanSurfaceSideCount^2)

    OceanSurface* m_Ocean[m_OceanSurfaceSideCount * m_OceanSurfaceSideCount] = { nullptr};
    //SkyBox m_SkyBox = SkyBox("SkyBox");
};

