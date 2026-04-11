#include "D3D11Application.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>

#include "TimeManager.h"
#include "ObjectManager.h"
#include "SceneManager.h"
#include "CameraManager.h"
#include "InputManager.h"
#include "OceanComputeManager.h"
#include "FFTManager.h"

using namespace DirectX;

D3D11Application* D3D11Application::m_Instance = nullptr;

D3D11Application::D3D11Application(const string& title)
    : WindowApplication(title)
{
}

D3D11Application::~D3D11Application()
{
    CleanupImGui();

	// Ensure all pointers are released
    m_d3dDeviceContext->Flush();
    SafeRelease(m_d3dRasterizerState);
    DestroySwapchainResources();
	DestroyDepthStencilResources();
    SafeRelease(m_d3dSwapChain);
    SafeRelease(m_d3dDXGIFactory);
    SafeRelease(m_d3dDeviceContext);
#if !defined(NDEBUG)
    m_d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
    SafeRelease(m_d3dDebug);
#endif
    SafeRelease(m_d3dDevice);
}

bool D3D11Application::InitializeInstance(const string& title)
{
    if (!m_Instance)
    {
        m_Instance = new D3D11Application(title);
        return true;
    }

    return false;
}

bool D3D11Application::Initialize()
{
    if (!WindowApplication::Initialize())
    {
        return false;
	}

    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_d3dDXGIFactory))))
    {
        cout << "Failed to create DXGI Factory." << endl;
        return false;
	}

	constexpr D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1;

    UINT deviceFlags = 0;
#if !defined(NDEBUG)
    deviceFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif

    ID3D11DeviceContext* deviceContext;

    if (FAILED(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        deviceFlags,
        &deviceFeatureLevel,
        1,
        D3D11_SDK_VERSION,
        &m_d3dDevice,
        nullptr,
        &deviceContext)))
    {
		cout << "D3D11: Failed to create device and device context." << endl;
        return false;
    }

#if !defined(NDEBUG)
    if (FAILED(m_d3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&m_d3dDebug))))
    {
        cout << "D3D11: Failed to get the debug layer from the device\n";
        return false;
    }
#endif
    
    m_d3dDeviceContext = deviceContext;

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

    if (FAILED(m_d3dDXGIFactory->CreateSwapChainForHwnd(
        m_d3dDevice,
        glfwGetWin32Window(GetWindow()),
        &swapChainDescriptor,
        &swapChainFullscreenDescriptor,
        nullptr,
        &m_d3dSwapChain)))
    {
        cout << "DXGI: Failed to create swapchain." << endl;
        return false;
    }

    if (!CreateSwapchainResources())
    {
        return false;
    }

    if (!CreateDepthStencilResources())
    {
        return false;
	}

    m_d3dViewport.TopLeftX = 0;
    m_d3dViewport.TopLeftY = 0;
    m_d3dViewport.Width = static_cast<float>(GetWindowWidth());
    m_d3dViewport.Height = static_cast<float>(GetWindowHeight());
    m_d3dViewport.MinDepth = 0.0f;
    m_d3dViewport.MaxDepth = 1.0f;

    if (!InitializeManagers())
    {
        return false;
    }

    if (!InitializeImGui())
    {
        return false;
    }

    return true;
}

bool D3D11Application::InitializeManagers()
{
    if (!TimeManager::Initialize())
    {
        return false;
    }

    if (!InputManager::Initialize())
    {
        return false;
    }

    if (!OceanComputeManager::Initialize())
    {
        return false;
    }

    if (!FFTManager::Initialize(OceanComputeManager::GetInstance().GetOceanTextureSize()))
    {
        return false;
    }

    if (!CameraManager::Initialize())
    {
        return false;
    }

    if (!ObjectManager::Initialize())
    {
        return false;
    }

    if (!SceneManager::Initialize())
    {
        return false;
    }

    if (!ObjectManager::GetInstance().InitializeObjects())
    {
        return false;
    }

    return true;
}

bool D3D11Application::InitializeImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOther(GetWindow(), true);
    ImGui_ImplDX11_Init(m_d3dDevice, m_d3dDeviceContext);

    return true;
}

void D3D11Application::CleanupImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool D3D11Application::CreateSwapchainResources()
{
    ID3D11Texture2D* backBuffer = nullptr;

    if (FAILED(m_d3dSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
    {
        cout << "D3D11: Failed to get back buffer from the SwapChain." << endl;
        return false;
    }

    if (FAILED(m_d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &m_d3dRenderTargetView)))
    {
        cout << "D3D11: Failed to create render target view for the back buffer." << endl;
        return false;
    }

	SafeRelease(backBuffer);

    return true;
}

bool D3D11Application::CreateDepthStencilResources()
{
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {};

	depthStencilBufferDesc.ArraySize = 1;
    depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilBufferDesc.CPUAccessFlags = 0;
	depthStencilBufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilBufferDesc.Width = GetWindowWidth();
	depthStencilBufferDesc.Height = GetWindowHeight();
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.SampleDesc.Quality = 0;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    if (FAILED(m_d3dDevice->CreateTexture2D(
        &depthStencilBufferDesc,
        nullptr,
        &m_d3dDepthStencilBuffer)))
    {
        cout << "D3D11: Failed to create depth stencil buffer texture." << endl;
        return false;
	}

    if (FAILED(m_d3dDevice->CreateDepthStencilView(
        m_d3dDepthStencilBuffer,
        nullptr,
        &m_d3dDepthStencilView)))
    {
        cout << "D3D11: Failed to create depth stencil view." << endl;
        return false;
    }

	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};

	depthStencilStateDesc.DepthEnable = TRUE;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilStateDesc.StencilEnable = FALSE;

    if (FAILED(m_d3dDevice->CreateDepthStencilState(
        &depthStencilStateDesc,
        &m_d3dDepthStencilState)))
    {
        cout << "D3D11: Failed to create depth stencil state." << endl;
        return false;
	}

	D3D11_RASTERIZER_DESC rasterizerDesc = {};

	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;

    if (FAILED(m_d3dDevice->CreateRasterizerState(
        &rasterizerDesc,
        &m_d3dRasterizerState)))
    {
        cout << "D3D11: Failed to create rasterizer state." << endl;
        return false;
    }

	return true;
}

bool D3D11Application::Load()
{
    SceneManager::GetInstance().Start();

    CameraManager::GetInstance().Start();

    OceanComputeManager::GetInstance().Start();

    ObjectManager::GetInstance().Start();

    return true;
}

void D3D11Application::DestroySwapchainResources()
{
    SafeRelease(m_d3dRenderTargetView);
}

void D3D11Application::DestroyDepthStencilResources()
{
    SafeRelease(m_d3dDepthStencilView);
    SafeRelease(m_d3dDepthStencilBuffer);
    SafeRelease(m_d3dDepthStencilState);
    SafeRelease(m_d3dRasterizerState);
}

void D3D11Application::ClearScreen(const float clearColor[4], float clearDepth, UINT8 clearStencil)
{
    m_d3dDeviceContext->ClearRenderTargetView(m_d3dRenderTargetView, clearColor);
    m_d3dDeviceContext->ClearDepthStencilView(
        m_d3dDepthStencilView,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        clearDepth,
        clearStencil);
}

void D3D11Application::Present(bool vSync)
{
    m_d3dSwapChain->Present(vSync ? 1 : 0, 0);
}

void D3D11Application::OnResize(const int32_t width, const int32_t height)
{
    WindowApplication::OnResize(width, height);
    m_d3dDeviceContext->Flush();

    DestroySwapchainResources();
    //DestroyDepthStencilResources();

    if (FAILED(m_d3dSwapChain->ResizeBuffers(
        0,
        width,
        height,
        DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM,
        0)))
    {
        cout << "D3D11: Failed to recreate swapchain buffers." << endl;
        return;
    }

    CreateSwapchainResources();
    //CreateDepthStencilResources();
}

void D3D11Application::Update()
{
    WindowApplication::Update();

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    UpdateManagers();
}

void D3D11Application::UpdateManagers()
{
    TimeManager::GetInstance().Update();
	OceanComputeManager::GetInstance().Update();
	SceneManager::GetInstance().Update();
    CameraManager::GetInstance().Update();
	ObjectManager::GetInstance().Update();
}

void D3D11Application::Render()
{
    constexpr float clearColor[] = { 0.53f, 0.81f, 0.92f, 1.0f };

	const int cascadeSize = OceanComputeManager::GetInstance().m_CascadeNumber;

	ClearScreen(Colors::SkyBlue, 1.0f, 0);

    // D3D11 rendering for all objects
    m_d3dDeviceContext->RSSetState(m_d3dRasterizerState);
    m_d3dDeviceContext->RSSetViewports(1, &m_d3dViewport);

    m_d3dDeviceContext->OMSetRenderTargets(1, &m_d3dRenderTargetView, m_d3dDepthStencilView);
    m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState, 1);

    // D3D11 rendering for each object
    for (int i = 0; i < ObjectManager::GetInstance().GetObjectList().size(); i++)
    {
        auto object = ObjectManager::GetInstance().GetObjectList()[i];

        m_d3dDeviceContext->IASetPrimitiveTopology(object->GetTopology());

        const UINT vertexStride = object->GetVertexStride();
        constexpr UINT vertexOffset = 0;

        m_d3dDeviceContext->IASetInputLayout(object->GetInputLayout());

        m_d3dDeviceContext->IASetVertexBuffers(0, 1, &object->GetVertexBuffer(), &vertexStride, &vertexOffset);

        m_d3dDeviceContext->IASetIndexBuffer(object->GetIndexBuffer(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

        m_d3dDeviceContext->VSSetShader(object->GetVertexShader(), nullptr, 0);
        m_d3dDeviceContext->VSSetConstantBuffers(0, 1, &object->GetConstantBuffers());

        if (object->UseTessellation())
        {
            m_d3dDeviceContext->HSSetShader(object->GetHullShader(), nullptr, 0);
            m_d3dDeviceContext->HSSetConstantBuffers(0, 1, &object->GetConstantBuffers());

            m_d3dDeviceContext->DSSetShader(object->GetDomainShader(), nullptr, 0);
            m_d3dDeviceContext->DSSetConstantBuffers(0, 1, &object->GetConstantBuffers());

            //ID3D11ShaderResourceView* oceanSRV[1] = { OceanComputeManager::GetInstance().GetDisplacementSRV() };
            m_d3dDeviceContext->DSSetShaderResources(0, cascadeSize, OceanComputeManager::GetInstance().GetDisplacementSRV());
            m_d3dDeviceContext->DSSetSamplers(0, 1, &object->GetSamplerState());
        }

        m_d3dDeviceContext->PSSetShader(object->GetPixelShader(), nullptr, 0);

        //ID3D11ShaderResourceView* pixelShaderSRV[1] = { OceanComputeManager::GetInstance().GetSlopeSRV() };
        m_d3dDeviceContext->PSSetShaderResources(0, cascadeSize, OceanComputeManager::GetInstance().GetSlopeSRV());
		m_d3dDeviceContext->PSSetSamplers(0, 1, &object->GetSamplerState());

        if (object->GetUpdatePixelShaderBuffer())
        {
            m_d3dDeviceContext->PSSetConstantBuffers(0, 1, &object->GetPixelShaderBuffers());
            object->SetUpdatePixelShaderBuffer(false);
        }

        m_d3dDeviceContext->DrawIndexed(object->GetIndexCount(), 0, 0);

        ID3D11ShaderResourceView* nullSRV[4] = { nullptr };
        m_d3dDeviceContext->DSSetShaderResources(0, cascadeSize, nullSRV);
        m_d3dDeviceContext->PSSetShaderResources(0, cascadeSize, nullSRV);
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    Present(true);
}

bool D3D11Application::CompileShader(const wstring& fileName, const string& entryPoint, const string& profile, ID3DBlob*& shaderBlob) const
{
    constexpr UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    // Debug layer

    ID3DBlob* tempShaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    
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
        cout << "D3D11: Failed to read shader from file\n";
		cout << fileName.data() << "\n";

        if (errorBlob != nullptr)
        {
            cout << "D3D11: With message: " << 
                static_cast<char*>(errorBlob->GetBufferPointer()) << "\n";
        }

        return false;
    }

    shaderBlob = tempShaderBlob;

    return true;
}

ID3D11VertexShader* D3D11Application::CreateVertexShader(const wstring& fileName, ID3DBlob*& vertexShaderBlob) const
{
    if (!CompileShader(fileName, "Main", "vs_5_0", vertexShaderBlob))
    {
        return nullptr;
    }

    ID3D11VertexShader* vertexShader;

    if (FAILED(m_d3dDevice->CreateVertexShader(
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        nullptr,
        &vertexShader)))
    {
        cout << "D3D11: Failed to compile vertex shader\n";
        return nullptr;
    }

    return vertexShader;
}

ID3D11PixelShader* D3D11Application::CreatePixelShader(const wstring& fileName) const
{
    ID3DBlob* pixelShaderBlob = nullptr;

    if (!CompileShader(fileName, "Main", "ps_5_0", pixelShaderBlob))
    {
        return nullptr;
    }

    ID3D11PixelShader* pixelShader;

    if (FAILED(m_d3dDevice->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize(),
        nullptr,
        &pixelShader)))
    {
        cout << "D3D11: Failed to compile pixel shader\n";
        return nullptr;
    }

    return pixelShader;
}

ID3D11HullShader* D3D11Application::CreateHullShader(const wstring& fileName) const
{
    ID3DBlob* hullShaderBlob = nullptr;

    if (!CompileShader(fileName, "Main", "hs_5_0", hullShaderBlob))
    {
        return nullptr;
    }

    ID3D11HullShader* hullShader;

    if (FAILED(m_d3dDevice->CreateHullShader(
        hullShaderBlob->GetBufferPointer(),
        hullShaderBlob->GetBufferSize(),
        nullptr,
        &hullShader)))
    {
        cout << "D3D11: Failed to compile hull shader\n";
        return nullptr;
    }

    return hullShader;
}

ID3D11DomainShader* D3D11Application::CreateDomainShader(const wstring& fileName) const
{
    ID3DBlob* shaderBlob = nullptr;

    if (!CompileShader(fileName, "Main", "ds_5_0", shaderBlob))
    {
        return nullptr;
    }

    ID3D11DomainShader* domainShader = nullptr;

    if (FAILED(m_d3dDevice->CreateDomainShader(
        shaderBlob->GetBufferPointer(), 
        shaderBlob->GetBufferSize(), 
        nullptr, 
        &domainShader)))
    {
        cout << "D3D11: Failed to create domain shader\n";
		return nullptr;
    }

    return domainShader;
}

ID3D11ComputeShader* D3D11Application::CreateComputeShader(const wstring& fileName) const
{
    ID3DBlob* shaderBlob = nullptr;

    if (!CompileShader(fileName, "Main", "cs_5_0", shaderBlob))
    {
        return nullptr;
    }

    ID3D11ComputeShader* computeShader = nullptr;

    if (FAILED(m_d3dDevice->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr,
        &computeShader)))
    {
        cout << "D3D11: Failed to create compute shader\n";
        return nullptr;
    }

    return computeShader;
}

ID3D11ComputeShader* D3D11Application::CreateComputeShaderWithEntry(const wstring& fileName, LPCSTR entryPoint) const
{
    ID3DBlob* shaderBlob = nullptr;

    if (!CompileShader(fileName, entryPoint, "cs_5_0", shaderBlob))
    {
        return nullptr;
    }

    ID3D11ComputeShader* computeShader;

    if (FAILED(m_d3dDevice->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr,
        &computeShader)))
    {
        cout << "D3D11: Failed to create compute shader\n";
        return nullptr;
    }

    return computeShader;
}