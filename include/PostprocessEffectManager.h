#pragma once

#include<vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

using namespace std;
using namespace DirectX;

class IPostprocessEffect;

// Define the vertex data structure
struct VertexDataPostprocessEffect
{
    XMFLOAT2 m_UV;
};

class PostprocessEffectManager
{
public:
    PostprocessEffectManager();
    ~PostprocessEffectManager();

    static PostprocessEffectManager& GetInstance()
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

    void AddEffectToList(IPostprocessEffect* effect);

    vector<IPostprocessEffect*>& GetEffectList() { return m_PostprocessEffects; }

    static bool Initialize();
    bool InitializeEffects();
	void Start();
    void Update();
    void Render();

    bool OnResize();

private:
    static PostprocessEffectManager* m_Instance;

    wstring m_VertexShaderFile;

    ID3D11InputLayout* m_d3dInputLayout = nullptr;
    ID3D11Buffer* m_d3dVertexBuffer = nullptr;
    ID3D11Buffer* m_d3dIndexBuffer = nullptr;

    ID3D11VertexShader* m_d3dVertexShader = nullptr;

    vector<VertexDataPostprocessEffect> m_Vertices;
    vector<int> m_Indices;

    ID3D11Texture2D* m_d3dPostprocessPingPongBuffer[2] = { nullptr };
    ID3D11RenderTargetView* m_d3dPostprocessPingPongRTV[2] = { nullptr };
    ID3D11ShaderResourceView* m_d3dPostprocessPingPongSRV[2] = { nullptr };

	void ReleaseResources();

    void GenerateMesh();

    bool CreateRenderTargetResources();
	void DestroyRenderTargetResources();

    UINT GetVerticesByteWidth() { return static_cast<UINT>(sizeof(VertexDataPostprocessEffect) * m_Vertices.size()); }
    UINT GetIndicesByteWidth() { return static_cast<UINT>(sizeof(int) * m_Indices.size()); }

    UINT GetIndexCount() { return static_cast<UINT>(m_Indices.size()); }

    UINT GetVertexStride() { return sizeof(VertexDataPostprocessEffect); }

    vector<IPostprocessEffect*> m_PostprocessEffects;

    template<typename T>
    inline void SafeRelease(T& ptr)
    {
        if (ptr != NULL)
        {
            ptr->Release();
            ptr = NULL;
        }
    }
};