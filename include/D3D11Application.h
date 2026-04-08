#pragma once

#include <d3d11.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_dx11.h"

#include "WindowApplication.h"

#include <vector>

class D3D11Application final : public WindowApplication
{
public:
    D3D11Application(const std::string& title);
    ~D3D11Application() override;

	static bool InitializeInstance(const std::string& title);

	static D3D11Application& GetInstance()
	{
		return *m_Instance;
	}

	ID3D11Device* GetDevice() const { return m_d3dDevice; }
	ID3D11DeviceContext* GetDeviceContext() const { return m_d3dDeviceContext; }

	bool CompileShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& profile, ID3DBlob*& shaderBlob) const;

	ID3D11VertexShader* CreateVertexShader(const std::wstring& fileName, ID3DBlob*& vertexShaderBlob) const;

	ID3D11PixelShader* CreatePixelShader(const std::wstring& fileName) const; 

	ID3D11HullShader* CreateHullShader(const std::wstring& fileName) const;
	ID3D11DomainShader* CreateDomainShader(const std::wstring& fileName) const;

	ID3D11ComputeShader* CreateComputeShader(const std::wstring& fileName) const;
	ID3D11ComputeShader* CreateComputeShaderWithEntry(const std::wstring& filePath, LPCSTR entryPoint) const;

protected:
	bool Initialize() override;
    bool Load() override;
    void OnResize(
	    int32_t width,
        int32_t height) override;
    void Render() override;
    void Update() override;

private:
	static D3D11Application* m_Instance;

	bool CreateSwapchainResources();
	void DestroySwapchainResources();
	bool CreateDepthStencilResources();
	void DestroyDepthStencilResources();

	void ClearScreen(const float clearColor[4], float clearDepth, UINT8 clearStencil);
	void Present(bool vSync);

	bool InitializeManagers();
	void UpdateManagers();

	bool InitializeImGui();
	void CleanupImGui();

	ID3D11Device* m_d3dDevice = nullptr;
	ID3D11DeviceContext* m_d3dDeviceContext = nullptr;
	IDXGIFactory2* m_d3dDXGIFactory = nullptr;
	IDXGISwapChain1* m_d3dSwapChain = nullptr;

	ID3D11RenderTargetView* m_d3dRenderTargetView = nullptr;
	ID3D11DepthStencilView* m_d3dDepthStencilView = nullptr;
	ID3D11Texture2D* m_d3dDepthStencilBuffer = nullptr;
	ID3D11DepthStencilState* m_d3dDepthStencilState = nullptr;
	
	ID3D11RasterizerState* m_d3dRasterizerState = nullptr;

	D3D11_VIEWPORT m_d3dViewport = { 0 };

	ID3D11Debug* m_d3dDebug = nullptr;

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