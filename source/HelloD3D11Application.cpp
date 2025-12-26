#include "HelloD3D11Application.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <DirectXColors.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

template <UINT TDebugNameLength>
inline void SetDebugName(_In_ ID3D11DeviceChild* deviceResource, _In_z_ const char(&debugName)[TDebugNameLength])
{
    deviceResource->SetPrivateData(WKPDID_D3DDebugObjectName, TDebugNameLength - 1, debugName);
}

using Position = DirectX::XMFLOAT3;
using Color = DirectX::XMFLOAT3;

struct VertexPositionColor
{
    Position position;
    Color color;
};

HelloD3D11Application::HelloD3D11Application(const std::string& title)
    : Application(title)
{
}

HelloD3D11Application::~HelloD3D11Application()
{
    _deviceContext->Flush();
    _triangleVertices.Reset();
    _rasterState.Reset();
    DestroySwapchainResources();
    _swapChain.Reset();
    _dxgiFactory.Reset();
    _deviceContext.Reset();
#if !defined(NDEBUG)
    _debug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
    _debug.Reset();
#endif
    _device.Reset();
}

bool HelloD3D11Application::Initialize()
{
    if (!Application::Initialize())
    {
        return false;
	}

    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory))))
    {
        std::cout << "Failed to create DXGI Factory." << std::endl;
        return false;
	}

	constexpr D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;

    UINT deviceFlags = 0;
#if !defined(NDEBUG)
    deviceFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif

    ComPtr<ID3D11DeviceContext> deviceContext;

    if (FAILED(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        deviceFlags,
        &deviceFeatureLevel,
        1,
        D3D11_SDK_VERSION,
        &_device,
        nullptr,
        &deviceContext)))
    {
		std::cout << "D3D11: Failed to create device and device context." << std::endl;
        return false;
    }

#if !defined(NDEBUG)
    if (FAILED(_device.As(&_debug)))
    {
        std::cout << "D3D11: Failed to get the debug layer from the device\n";
        return false;
    }
#endif
    
    _deviceContext = deviceContext;

    constexpr char deviceName[] = "DEV_Main";
    _device->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(deviceName), deviceName);
    SetDebugName(_deviceContext.Get(), "CTX_Main");

	DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor = {};
    swapChainDescriptor.Width = GetWindowWidth();
	swapChainDescriptor.Height = GetWindowHeight();
	swapChainDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDescriptor.SampleDesc.Count = 1;
	swapChainDescriptor.SampleDesc.Quality = 0;
	swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDescriptor.BufferCount = 2;
	swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDescriptor.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
    swapChainDescriptor.Flags = {};

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDescriptor = {};
    swapChainFullscreenDescriptor.Windowed = true;

    if (FAILED(_dxgiFactory->CreateSwapChainForHwnd(
        _device.Get(),
        glfwGetWin32Window(GetWindow()),
        &swapChainDescriptor,
        &swapChainFullscreenDescriptor,
        nullptr,
        & _swapChain)))
    {
        std::cout << "DXGI: Failed to create swapchain." << std::endl;
        return false;
    }

    if (!CreateSwapchainResources())
    {
        return false;
    }

    return true;
}

bool HelloD3D11Application::Load()
{
    ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
    _vertexShader = CreateVertexShader(L"assets/shaders/Main.vs.hlsl", vertexShaderBlob);

    if (_vertexShader == nullptr)
    {
        return false;
    }

    _pixelShader = CreatePixelShader(L"assets/shaders/Main.ps.hlsl");

    if (_pixelShader == nullptr)
    {
        return false;
    }

    constexpr D3D11_INPUT_ELEMENT_DESC vertexInputLayoutInfo[] =
    {
        {
            "POSITION",
            0,
            DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            offsetof(VertexPositionColor, position),
            D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        {
            "COLOR",
            0,
            DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            offsetof(VertexPositionColor, color),
            D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
    };

    if (FAILED(_device->CreateInputLayout(
        vertexInputLayoutInfo,
        _countof(vertexInputLayoutInfo),
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        &_vertexLayout)))
    {
        std::cout << "D3D11: Failed to create default vertex input layout\n";
        return false;
    }

    constexpr VertexPositionColor vertices[] =
    {
        { Position { 0.0f, 0.5f, 0.0f }, Color { 0.25f, 0.39f, 0.19f } },
        { Position { 0.5f, -0.5f, 0.0f }, Color { 0.44f, 0.75f, 0.35f } },
        { Position { -0.5f, -0.5f, 0.0f }, Color { 0.38f, 0.55f, 0.20f } }
    };

    D3D11_BUFFER_DESC bufferInfo = {};
    bufferInfo.ByteWidth = sizeof(vertices);
    bufferInfo.Usage = D3D11_USAGE_IMMUTABLE;
    bufferInfo.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA resourceData = {};
    resourceData.pSysMem = vertices;

    if (FAILED(_device->CreateBuffer(
        &bufferInfo,
        &resourceData,
        &_triangleVertices)))
    {
        std::cout << "D3D11: Failed to create triangle vertex buffer\n";
        return false;
    }

    return true;
}

bool HelloD3D11Application::CreateSwapchainResources()
{
    ComPtr<ID3D11Texture2D> backBuffer = nullptr;

    if (FAILED(_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
    {
        std::cout << "D3D11: Failed to get back buffer from the SwapChain." << std::endl;
        return false;
    }

    if (FAILED(_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &_renderTarget)))
    {
        std::cout << "D3D11: Failed to create render target view for the back buffer." << std::endl;
        return false;
    }

    return true;
}

void HelloD3D11Application::DestroySwapchainResources()
{
    _renderTarget.Reset();
}

void HelloD3D11Application::OnResize(const int32_t width, const int32_t height)
{
    Application::OnResize(width, height);
    _deviceContext->Flush();

    DestroySwapchainResources();

    if (FAILED(_swapChain->ResizeBuffers(
        0,
        width,
        height,
        DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM,
        0)))
    {
        std::cout << "D3D11: Failed to recreate swapchain buffers." << std::endl;
        return;
    }

    CreateSwapchainResources();
}

void HelloD3D11Application::Update()
{
    Application::Update();
}

void HelloD3D11Application::Render()
{
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(GetWindowWidth());
    viewport.Height = static_cast<float>(GetWindowHeight());
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    constexpr float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    constexpr UINT vertexStride = sizeof(VertexPositionColor);
    constexpr UINT vertexOffset = 0;

    _deviceContext->ClearRenderTargetView(_renderTarget.Get(), clearColor);

    _deviceContext->IASetInputLayout(_vertexLayout.Get());

    _deviceContext->IASetVertexBuffers(
        0,
        1,
        _triangleVertices.GetAddressOf(),
        &vertexStride,
        &vertexOffset);

    _deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    _deviceContext->VSSetShader(_vertexShader.Get(), nullptr, 0);

    _deviceContext->RSSetViewports(1, &viewport);

    _deviceContext->PSSetShader(_pixelShader.Get(), nullptr, 0);

    _deviceContext->OMSetRenderTargets(1, _renderTarget.GetAddressOf(), nullptr);

    _deviceContext->Draw(3, 0);

    _swapChain->Present(1, 0);
}

bool HelloD3D11Application::CompileShader(
    const std::wstring& fileName,
    const std::string& entryPoint,
    const std::string& profile,
    ComPtr<ID3DBlob>& shaderBlob) const
{
    constexpr UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

    ComPtr<ID3DBlob> tempShaderBlob = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    
    if (FAILED(D3DCompileFromFile(
        fileName.data(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.data(),
        profile.data(),
        compileFlags,
        0,
        &tempShaderBlob,
        &errorBlob)))
    {
        std::cout << "D3D11: Failed to read shader from file\n";

        if (errorBlob != nullptr)
        {
            std::cout << "D3D11: With message: " << 
                static_cast<char*>(errorBlob->GetBufferPointer()) << "\n";
        }

        return false;
    }

    shaderBlob = std::move(tempShaderBlob);

    return true;
}

HelloD3D11Application::ComPtr<ID3D11VertexShader> HelloD3D11Application::CreateVertexShader(
    const std::wstring& fileName,
    ComPtr <ID3DBlob>& vertexShaderBlob) const
{
    if (!CompileShader(fileName, "Main", "vs_5_0", vertexShaderBlob))
    {
        return nullptr;
    }

    ComPtr<ID3D11VertexShader> vertexShader;

    if (FAILED(_device->CreateVertexShader(
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        nullptr,
        &vertexShader)))
    {
        std::cout << "D3D11: Failed to compile vertex shader\n";
        return nullptr;
    }

    return vertexShader;
}

HelloD3D11Application::ComPtr<ID3D11PixelShader> HelloD3D11Application::CreatePixelShader(const std::wstring& fileName) const
{
    ComPtr<ID3DBlob> pixelShaderBlob = nullptr;

    if (!CompileShader(fileName, "Main", "ps_5_0", pixelShaderBlob))
    {
        return nullptr;
    }

    ComPtr<ID3D11PixelShader> pixelShader;

    if (FAILED(_device->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize(),
        nullptr,
        &pixelShader)))
    {
        std::cout << "D3D11: Failed to compile pixel shader\n";
        return nullptr;
    }

    return pixelShader;
}