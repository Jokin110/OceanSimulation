#include "D3D11Application.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>

using namespace DirectX;
using std::vector;

// Define the vertex data structure
struct VertexData
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT3 color;
};

/*
// Define the vertices of the mesh
VertexData m_Vertices[4] =
{
    { XMFLOAT3(-1.0f, 0.0f,  -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 0
    { XMFLOAT3(-1.0f,  0.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 1
    { XMFLOAT3(1.0f,  0.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 2
    { XMFLOAT3(1.0f, 0.0f,  -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }, // 3
};

// Define the indices of the mesh
int m_Indices[6] =
{
    0, 1, 2,
    0, 2, 3
};
*/

vector<VertexData> m_Vertices;
vector<int> m_Indices;

// Define the structure of the constant buffers for the vertex shader
struct PerObjectConstantBufferData
{
    XMMATRIX m_WorldMatrix;
	XMMATRIX m_InverseTransposeWorldMatrix;
    XMMATRIX m_ViewProjectionMatrix;
	float m_Time;
	XMFLOAT3 m_CameraPosition;
};

XMFLOAT3 m_CameraPosition = XMFLOAT3(0.0f, 15.0f, -20.0f);
XMFLOAT3 m_CameraFocusPoint = XMFLOAT3(0.0f, 0.0f, 0.0f);

D3D11Application::D3D11Application(const std::string& title)
    : WindowApplication(title)
{
}

D3D11Application::~D3D11Application()
{
	// Ensure all pointers are released
    m_d3dDeviceContext->Flush();
	m_d3dConstantBuffers.Reset();
	m_d3dIndexBuffer.Reset();
    m_d3dVertexBuffer.Reset();
	m_d3dInputLayout.Reset();
	m_d3dVertexShader.Reset();
	m_d3dPixelShader.Reset();
    m_d3dRasterizerState.Reset();
    DestroySwapchainResources();
	DestroyDepthStencilResources();
    m_d3dSwapChain.Reset();
    m_d3dDXGIFactory.Reset();
    m_d3dDeviceContext.Reset();
#if !defined(NDEBUG)
    m_d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
    m_d3dDebug.Reset();
#endif
    m_d3dDevice.Reset();
}

bool D3D11Application::Initialize()
{
    if (!WindowApplication::Initialize())
    {
        return false;
	}

    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_d3dDXGIFactory))))
    {
        std::cout << "Failed to create DXGI Factory." << std::endl;
        return false;
	}

	constexpr D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1;

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
        &m_d3dDevice,
        nullptr,
        &deviceContext)))
    {
		std::cout << "D3D11: Failed to create device and device context." << std::endl;
        return false;
    }

#if !defined(NDEBUG)
    if (FAILED(m_d3dDevice.As(&m_d3dDebug)))
    {
        std::cout << "D3D11: Failed to get the debug layer from the device\n";
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
        m_d3dDevice.Get(),
        glfwGetWin32Window(GetWindow()),
        &swapChainDescriptor,
        &swapChainFullscreenDescriptor,
        nullptr,
        &m_d3dSwapChain)))
    {
        std::cout << "DXGI: Failed to create swapchain." << std::endl;
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

    return true;
}

bool D3D11Application::CreateSwapchainResources()
{
    ComPtr<ID3D11Texture2D> backBuffer = nullptr;

    if (FAILED(m_d3dSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
    {
        std::cout << "D3D11: Failed to get back buffer from the SwapChain." << std::endl;
        return false;
    }

    if (FAILED(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_d3dRenderTargetView)))
    {
        std::cout << "D3D11: Failed to create render target view for the back buffer." << std::endl;
        return false;
    }

	backBuffer.Reset();

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
        std::cout << "D3D11: Failed to create depth stencil buffer texture." << std::endl;
        return false;
	}

    if (FAILED(m_d3dDevice->CreateDepthStencilView(
        m_d3dDepthStencilBuffer.Get(),
        nullptr,
        &m_d3dDepthStencilView)))
    {
        std::cout << "D3D11: Failed to create depth stencil view." << std::endl;
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
        std::cout << "D3D11: Failed to create depth stencil state." << std::endl;
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
        std::cout << "D3D11: Failed to create rasterizer state." << std::endl;
        return false;
    }

	return true;
}

bool D3D11Application::Load()
{
    ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
#if _DEBUG
    m_d3dVertexShader = CreateVertexShader(L"assets/shaders/GerstnerWavesVS.hlsl", vertexShaderBlob);
#else
	m_d3dVertexShader = CreateVertexShader(L"../../assets/shaders/SumOfSinesVS.hlsl", vertexShaderBlob);
#endif

    if (m_d3dVertexShader == nullptr)
    {
        return false;
    }

#if _DEBUG
    m_d3dPixelShader = CreatePixelShader(L"assets/shaders/PixelShader.hlsl");
#else
	m_d3dPixelShader = CreatePixelShader(L"../../assets/shaders/PixelShader.hlsl");
#endif

    if (m_d3dPixelShader == nullptr)
    {
        return false;
    }

    constexpr D3D11_INPUT_ELEMENT_DESC vertexInputLayout[] =
    {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            offsetof(VertexData, position),
            D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        {
            "NORMAL",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
			0,
            offsetof(VertexData, normal),
            D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
			0
        },
        {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            offsetof(VertexData, color),
            D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
    };

    if (FAILED(m_d3dDevice->CreateInputLayout(
        vertexInputLayout,
        _countof(vertexInputLayout),
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        &m_d3dInputLayout)))
    {
        std::cout << "D3D11: Failed to create default vertex input layout\n";
        return false;
    }

    GeneratePlaneMesh();

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(VertexData) * m_Vertices.size());
	vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA resourceData = {};
    resourceData.pSysMem = m_Vertices.data();

    if (FAILED(m_d3dDevice->CreateBuffer(
        &vertexBufferDesc,
        &resourceData,
        &m_d3dVertexBuffer)))
    {
        std::cout << "D3D11: Failed to create triangle vertex buffer\n";
        return false;
    }

	D3D11_BUFFER_DESC indexBufferDesc = {};

	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(int) * m_Indices.size());
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	resourceData.pSysMem = m_Indices.data();

    if (FAILED(m_d3dDevice->CreateBuffer(
        &indexBufferDesc,
        &resourceData,
        &m_d3dIndexBuffer)))
    {
		std::cout << "D3D11: Failed to create triangle index buffer\n";
		return false;
    }

	D3D11_BUFFER_DESC constantBufferDesc = {};

	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(PerObjectConstantBufferData);
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    if (FAILED(m_d3dDevice->CreateBuffer(
        &constantBufferDesc,
        nullptr,
        &m_d3dConstantBuffers)))
    {
        std::cout << "D3D11: Failed to create per-object constant buffer\n";
        return false;
	}

    return true;
}

void D3D11Application::DestroySwapchainResources()
{
    m_d3dRenderTargetView.Reset();
}

void D3D11Application::DestroyDepthStencilResources()
{
    m_d3dDepthStencilView.Reset();
    m_d3dDepthStencilBuffer.Reset();
    m_d3dDepthStencilState.Reset();
    m_d3dRasterizerState.Reset();
}

void D3D11Application::ClearScreen(const float clearColor[4], float clearDepth, UINT8 clearStencil)
{
    m_d3dDeviceContext->ClearRenderTargetView(m_d3dRenderTargetView.Get(), clearColor);
    m_d3dDeviceContext->ClearDepthStencilView(
        m_d3dDepthStencilView.Get(),
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
        std::cout << "D3D11: Failed to recreate swapchain buffers." << std::endl;
        return;
    }

    CreateSwapchainResources();
    //CreateDepthStencilResources();
}

void D3D11Application::Update()
{
    WindowApplication::Update();

    float cameraSpeed = 5.0f;

	XMFLOAT3 cameraForward = XMFLOAT3(m_CameraFocusPoint.x - m_CameraPosition.x, 0, m_CameraFocusPoint.z - m_CameraPosition.z);
    float length = sqrtf(cameraForward.x * cameraForward.x + cameraForward.y * cameraForward.y + cameraForward.z * cameraForward.z);
    cameraForward = XMFLOAT3(cameraForward.x / length, 0, cameraForward.z / length);
	XMFLOAT3 cameraRight = XMFLOAT3(cameraForward.z, 0, -cameraForward.x);

    if (glfwGetKey(GetWindow(), GLFW_KEY_W) == GLFW_PRESS)
    {
        m_CameraPosition.x += cameraSpeed * m_DeltaTime * cameraForward.x;
        m_CameraPosition.z += cameraSpeed * m_DeltaTime * cameraForward.z;
    }
	if (glfwGetKey(GetWindow(), GLFW_KEY_S) == GLFW_PRESS)
    {
        m_CameraPosition.x -= cameraSpeed * m_DeltaTime * cameraForward.x;
        m_CameraPosition.z -= cameraSpeed * m_DeltaTime * cameraForward.z;
    }
    if (glfwGetKey(GetWindow(), GLFW_KEY_A) == GLFW_PRESS)
    {
        m_CameraPosition.x -= cameraSpeed * m_DeltaTime * cameraRight.x;
        m_CameraPosition.z -= cameraSpeed * m_DeltaTime * cameraRight.z;
    }
    if (glfwGetKey(GetWindow(), GLFW_KEY_D) == GLFW_PRESS)
    {
        m_CameraPosition.x += cameraSpeed * m_DeltaTime * cameraRight.x;
        m_CameraPosition.z += cameraSpeed * m_DeltaTime * cameraRight.z;
    }

	XMVECTOR eyePosition = XMLoadFloat3(&m_CameraPosition);
	XMVECTOR focusPosition = XMVectorSet(m_CameraFocusPoint.x, m_CameraFocusPoint.y, m_CameraFocusPoint.z, 1.0f);
	XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX viewMatrix = XMMatrixLookAtLH(eyePosition, focusPosition, upDirection);
    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(45.0f),
        static_cast<float>(GetWindowWidth()) / static_cast<float>(GetWindowHeight()),
        0.1f,
		1000.0f);
	XMMATRIX worldMatrix = XMMatrixIdentity();

	PerObjectConstantBufferData constantBufferData = {};
    constantBufferData.m_WorldMatrix = worldMatrix;
	constantBufferData.m_InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));
    constantBufferData.m_ViewProjectionMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);
    constantBufferData.m_Time = m_Time;
	constantBufferData.m_CameraPosition = m_CameraPosition;

    m_d3dDeviceContext->UpdateSubresource(
        m_d3dConstantBuffers.Get(),
        0,
        nullptr,
        &constantBufferData,
        0,
		0);
}

void D3D11Application::Render()
{
    constexpr float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    constexpr UINT vertexStride = sizeof(VertexData);
    constexpr UINT vertexOffset = 0;

	ClearScreen(Colors::SkyBlue, 1.0f, 0);

    m_d3dDeviceContext->IASetInputLayout(m_d3dInputLayout.Get());

    m_d3dDeviceContext->IASetVertexBuffers(
        0,
        1,
        m_d3dVertexBuffer.GetAddressOf(),
        &vertexStride,
        &vertexOffset);

    m_d3dDeviceContext->IASetIndexBuffer(
        m_d3dIndexBuffer.Get(),
        DXGI_FORMAT::DXGI_FORMAT_R32_UINT,
		0);

    m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_d3dDeviceContext->VSSetShader(m_d3dVertexShader.Get(), nullptr, 0);
	m_d3dDeviceContext->VSSetConstantBuffers(0, 1, m_d3dConstantBuffers.GetAddressOf());

	m_d3dDeviceContext->RSSetState(m_d3dRasterizerState.Get());
    m_d3dDeviceContext->RSSetViewports(1, &m_d3dViewport);

    m_d3dDeviceContext->PSSetShader(m_d3dPixelShader.Get(), nullptr, 0);

    m_d3dDeviceContext->OMSetRenderTargets(1, m_d3dRenderTargetView.GetAddressOf(), m_d3dDepthStencilView.Get());
	m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState.Get(), 1);

    m_d3dDeviceContext->DrawIndexed(static_cast<UINT>(m_Indices.size()), 0, 0);

    Present(true);
}

bool D3D11Application::CompileShader(
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

D3D11Application::ComPtr<ID3D11VertexShader> D3D11Application::CreateVertexShader(
    const std::wstring& fileName,
    ComPtr <ID3DBlob>& vertexShaderBlob) const
{
    if (!CompileShader(fileName, "Main", "vs_5_0", vertexShaderBlob))
    {
        return nullptr;
    }

    ComPtr<ID3D11VertexShader> vertexShader;

    if (FAILED(m_d3dDevice->CreateVertexShader(
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

D3D11Application::ComPtr<ID3D11PixelShader> D3D11Application::CreatePixelShader(const std::wstring& fileName) const
{
    ComPtr<ID3DBlob> pixelShaderBlob = nullptr;

    if (!CompileShader(fileName, "Main", "ps_5_0", pixelShaderBlob))
    {
        return nullptr;
    }

    ComPtr<ID3D11PixelShader> pixelShader;

    if (FAILED(m_d3dDevice->CreatePixelShader(
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

void D3D11Application::GeneratePlaneMesh()
{
	m_Vertices.clear();
	m_Indices.clear();

    int widthVertices = 1024;
	int depthVertices = 1024;

    float separation = 0.1f;

	float startX = -((widthVertices - 1) * separation) / 2.0f;
    float startZ = -((depthVertices - 1) * separation) / 2.0f;

    for (int x = 0; x < widthVertices; x++)
    {
        for (int z = 0; z < depthVertices; z++)
        {
            VertexData vertex = VertexData {};

			vertex.position.x = startX + x * separation;
            vertex.position.y = 0.0f;
			vertex.position.z = startZ + z * separation;

			vertex.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

			vertex.color = XMFLOAT3(0.0f, 0.41f, 0.58f);

			m_Vertices.push_back(vertex);
        }
    }

    for (int x = 0; x < widthVertices - 1; x++)
    {
        for (int z = 0; z < depthVertices - 1; z++)
        {
            int bottomLeftVertex = x * depthVertices + z;
            int bottomRightVertex = (x + 1) * depthVertices + z;
			int topLeftVertex = bottomLeftVertex + 1;
            int topRightVertex = bottomRightVertex + 1;

            m_Indices.push_back(bottomLeftVertex);
            m_Indices.push_back(topLeftVertex);
            m_Indices.push_back(topRightVertex);

			m_Indices.push_back(bottomLeftVertex);
            m_Indices.push_back(topRightVertex);
			m_Indices.push_back(bottomRightVertex);
        }
    }
}