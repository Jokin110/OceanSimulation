#pragma once

#include "OceanSurface.h"
#include "SkyBox.h"
#include <DirectXMath.h>

const int OCEAN_SURFACE_SIDE_COUNT = 1;

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    static SceneManager& GetInstance()
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

    bool RegenerateMeshes();

	float GetFoamBias() const { return m_PixelShaderSettings.m_FoamBias; }
	float GetDecayFactor() const { return m_PixelShaderSettings.m_DecayFactor; }
    float GetFoamAddition() const { return m_PixelShaderSettings.m_FoamAddition; }

private:
    static SceneManager* m_Instance;

    PixelShaderConstantBufferData m_PixelShaderSettings;

    OceanSurface* m_Ocean[OCEAN_SURFACE_SIDE_COUNT * OCEAN_SURFACE_SIDE_COUNT] = { nullptr };
    SkyBox* m_SkyBox = nullptr;
};

