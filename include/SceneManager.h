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

	OceanSurface m_Ocean = OceanSurface("Ocean Surface");
    SkyBox m_SkyBox = SkyBox("SkyBox");
};

