#pragma once

#include <DirectXMath.h>
#include "OceanSurface.h"
#include "SkyBox.h"
#include "FogPostprocessEffect.h"

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
    void UpdateUI();

    bool RegenerateMeshes();

    ID3D11ShaderResourceView* const* GetSkyboxSRV() { return m_SkyBox->GetSkyboxSRV(); }

	float GetFoamBias() const { return m_PixelShaderSettings.m_FoamBias; }
	float GetDecayFactor() const { return m_PixelShaderSettings.m_DecayFactor; }
    float GetFoamAddition() const { return m_PixelShaderSettings.m_FoamAddition; }

	XMFLOAT3 GetLightDirection() const { return m_PixelShaderSettings.m_LightDirection; }
	XMFLOAT3 GetLightColor() const { return m_PixelShaderSettings.m_LightColor; }

    XMFLOAT3 GetSunColor() const { return m_SunSettings.m_SunColor; }
    float GetSunExponent() const { return m_SunSettings.m_SunExponent; }
    float GetSunBias() const { return m_SunSettings.m_SunBias; }

private:
    static SceneManager* m_Instance;

    PixelShaderConstantBufferData m_PixelShaderSettings;
    PerObjectPixelShaderConstantBufferDataSkyBox m_SunSettings;

    OceanSurface* m_Ocean[OCEAN_SURFACE_SIDE_COUNT * OCEAN_SURFACE_SIDE_COUNT] = { nullptr };
    SkyBox* m_SkyBox = nullptr;

    FogPostprocessEffect* m_FogPostProcessEffect = nullptr;

    string m_SceneSettingsSavePath = "Scenes";

    vector<string> m_SceneFolders;      // You will populate this by scanning m_SceneSettingsSavePath
    int m_SelectedSceneIndex = 0;       // Keeps track of the currently selected dropdown item
    char m_SceneNameInput[256] = "";    // Buffer for the text input field

    void SaveSettings(string parentPath);
    void LoadSettings(string parentPath);

    void LoadSceneNames();

    void SaveSceneSettings();
    void LoadSceneSettings();
};

