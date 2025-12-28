#pragma once

#include <d3d11.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

#include "WindowApplication.h"

class D3D11Application final : public WindowApplication
{
    template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
    D3D11Application(const std::string& title);
    ~D3D11Application() override;

protected:
	bool Initialize() override;
    bool Load() override;
    void OnResize(
	    int32_t width,
        int32_t height) override;
    void Render() override;
    void Update() override;

private:
	bool CreateSwapchainResources();
	void DestroySwapchainResources();
	bool CreateDepthStencilResources();
	void DestroyDepthStencilResources();

	void ClearScreen(const float clearColor[4], float clearDepth, UINT8 clearStencil);
	void Present(bool vSync);

	ComPtr<ID3D11Device> m_d3dDevice = nullptr;
	ComPtr<ID3D11DeviceContext> m_d3dDeviceContext = nullptr;
	ComPtr<IDXGIFactory2> m_d3dDXGIFactory = nullptr;
	ComPtr<IDXGISwapChain1> m_d3dSwapChain = nullptr;

	ComPtr<ID3D11RenderTargetView> m_d3dRenderTargetView = nullptr;
	ComPtr<ID3D11DepthStencilView> m_d3dDepthStencilView = nullptr;
	ComPtr<ID3D11Texture2D> m_d3dDepthStencilBuffer = nullptr;
	ComPtr<ID3D11DepthStencilState> m_d3dDepthStencilState = nullptr;
	
	ComPtr<ID3D11RasterizerState> m_d3dRasterizerState = nullptr;
	
	ComPtr<ID3D11InputLayout> m_d3dInputLayout = nullptr;
	ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
	ComPtr<ID3D11Buffer> m_d3dIndexBuffer = nullptr;

	ComPtr<ID3D11VertexShader> m_d3dVertexShader = nullptr;
	ComPtr<ID3D11PixelShader> m_d3dPixelShader = nullptr;

	ComPtr<ID3D11Buffer> m_d3dConstantBuffers = nullptr;

	D3D11_VIEWPORT m_d3dViewport = { 0 };

	ComPtr<ID3D11Debug> m_d3dDebug = nullptr;
	
	bool CompileShader(
		const std::wstring& fileName,
		const std::string& entryPoint,
		const std::string& profile,
		ComPtr<ID3DBlob>& shaderBlob) const;


	[[nodiscard]] ComPtr<ID3D11VertexShader> CreateVertexShader(
		const std::wstring& fileName,
		ComPtr<ID3DBlob>& vertexShaderBlob) const;

	[[nodiscard]] ComPtr<ID3D11PixelShader> CreatePixelShader(const std::wstring& fileName) const;
};