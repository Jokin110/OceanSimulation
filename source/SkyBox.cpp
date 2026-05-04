#include "SkyBox.h"
#include "CameraManager.h"

SkyBox::SkyBox(string name) : Object(name)
{
    m_VertexShaderFile = m_VertexShaderFilePath;
	m_PixelShaderFile = m_PixelShaderFilePath;

	m_SkyBoxTexture = new Texture2D(SKYBOX_TEXTURE_COUNT, m_SkyBoxTextureFilePath);

    m_PixelShaderSRVCount = SKYBOX_TEXTURE_COUNT;
}

SkyBox::~SkyBox()
{
    if (m_SkyBoxTexture)
    {
        delete m_SkyBoxTexture;
        m_SkyBoxTexture = nullptr;
    }
}

bool SkyBox::Initialize()
{
    bool result = Object::Initialize();

    m_Initialized = false;

    result = result && m_SkyBoxTexture->Initialize();

    if (m_d3dSamplerState)
    {
        m_d3dSamplerState->Release();
        m_d3dSamplerState = nullptr;
    }

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 0.0f;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateSamplerState(
        &samplerDesc,
        &m_d3dSamplerState)))
    {
        cout << "D3D11: Failed to create sampler state\n";
        return false;
    }

    m_Initialized = true;

	return result;
}

void SkyBox::Start()
{
	Object::Start();

    m_Scale *= 1400.0f;
}

void SkyBox::Update()
{
	m_Position = CameraManager::GetInstance().GetCameraPosition();
    m_Position.y = -100.0f;

	XMMATRIX scaleMatrix = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_Rotation.x), XMConvertToRadians(m_Rotation.y), XMConvertToRadians(m_Rotation.z));
	XMMATRIX translationMatrix = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

	XMMATRIX worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	m_VertexShaderConstantBufferData = {};
    m_VertexShaderConstantBufferData.m_WorldMatrix = worldMatrix;
    m_VertexShaderConstantBufferData.m_ViewProjectionMatrix = XMMatrixMultiply(CameraManager::GetInstance().GetViewMatrix(), CameraManager::GetInstance().GetProjectionMatrix());

	Object::Update();
}

UINT SkyBox::GetVertexInputLayout(D3D11_INPUT_ELEMENT_DESC*& inputLayout)
{
	static D3D11_INPUT_ELEMENT_DESC vertexInputLayout[] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(VertexDataSkyBox, m_Position),
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
        {
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            offsetof(VertexDataSkyBox, m_UV),
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        {
            "TEXCOORD",
            1,
            DXGI_FORMAT_R32_UINT,
            0,
            offsetof(VertexDataSkyBox, m_TextureIndex),
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
	};

	inputLayout = vertexInputLayout;

	return _countof(vertexInputLayout);
}

void SkyBox::GenerateMesh()
{
	m_Vertices.clear();
	m_Indices.clear();

    // Size is 1.0f, so we go from -0.5f to 0.5f
    float halfSize = 0.5f;

    // Define the UV coordinates for a face
    XMFLOAT2 uvTopLeft = XMFLOAT2(1.0f, 0.0f);
    XMFLOAT2 uvTopRight = XMFLOAT2(0.0f, 0.0f);
    XMFLOAT2 uvBotLeft = XMFLOAT2(1.0f, 1.0f);
    XMFLOAT2 uvBotRight = XMFLOAT2(0.0f, 1.0f);

    // Helper to push a face (4 vertices) and 2 triangles (6 indices)
    // We pass vertices in standard "reading order" (TopLeft, TopRight, BotLeft, BotRight)
    // But we generate indices to face INWARDS.
    auto AddFace = [&](XMFLOAT3 tl, XMFLOAT3 tr, XMFLOAT3 bl, XMFLOAT3 br, UINT index)
    {
        // Current index offset
        int startIdx = static_cast<int>(m_Vertices.size());

        // Add 4 vertices for this face
        m_Vertices.push_back({ tl, uvTopLeft, index });  // 0: Top-Left
        m_Vertices.push_back({ tr, uvTopRight, index }); // 1: Top-Right
        m_Vertices.push_back({ bl, uvBotLeft, index });  // 2: Bottom-Left
        m_Vertices.push_back({ br, uvBotRight, index }); // 3: Bottom-Right

        // Add indices for Inward facing triangles
        // Standard outward would be (0,1,2) and (2,1,3)
        // We swap them to flip the normal inwards: (0, 2, 1) and (2, 3, 1)
        m_Indices.push_back(startIdx + 0);
        m_Indices.push_back(startIdx + 2);
        m_Indices.push_back(startIdx + 1);

        m_Indices.push_back(startIdx + 2);
        m_Indices.push_back(startIdx + 3);
        m_Indices.push_back(startIdx + 1);
    };

    // Define corners
    XMFLOAT3 p0 = { -halfSize,  halfSize, -halfSize }; // Front Top Left
    XMFLOAT3 p1 = { halfSize,  halfSize, -halfSize }; // Front Top Right
    XMFLOAT3 p2 = { -halfSize, -halfSize, -halfSize }; // Front Bot Left
    XMFLOAT3 p3 = { halfSize, -halfSize, -halfSize }; // Front Bot Right
    XMFLOAT3 p4 = { -halfSize,  halfSize,  halfSize }; // Back Top Left
    XMFLOAT3 p5 = { halfSize,  halfSize,  halfSize }; // Back Top Right
    XMFLOAT3 p6 = { -halfSize, -halfSize,  halfSize }; // Back Bot Left
    XMFLOAT3 p7 = { halfSize, -halfSize,  halfSize }; // Back Bot Right

    // +Z Face (Back)
    AddFace(p5, p4, p7, p6, 0);

    // -Z Face (Front)
    AddFace(p0, p1, p2, p3, 1);

    // +X Face (Right)
    AddFace(p1, p5, p3, p7, 2);

    // -X Face (Left)
    AddFace(p4, p0, p6, p2, 3);

    // +Y Face (Top)
    AddFace(p4, p5, p0, p1, 4);

    // -Y Face (Bottom)
    AddFace(p2, p3, p6, p7, 5);
}

ID3D11ShaderResourceView* const* SkyBox::GetPixelShaderSRVs()
{
    return m_SkyBoxTexture->GetTextureSRVs();
}