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
    m_VertexShaderConstantBufferData.m_WorldViewProjectionMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(CameraManager::GetInstance().GetViewMatrix(), CameraManager::GetInstance().GetProjectionMatrix()));

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
		}
	};

	inputLayout = vertexInputLayout;

	return _countof(vertexInputLayout);
}

void SkyBox::GenerateMesh()
{
    m_Vertices.clear();
    m_Indices.clear();

    // Define the 8 unique corners of the cube
    m_Vertices.push_back({ XMFLOAT3(-1.0f,  1.0f, -1.0f) }); // 0: Front Top Left
    m_Vertices.push_back({ XMFLOAT3(1.0f,  1.0f, -1.0f) }); // 1: Front Top Right
    m_Vertices.push_back({ XMFLOAT3(-1.0f, -1.0f, -1.0f) }); // 2: Front Bot Left
    m_Vertices.push_back({ XMFLOAT3(1.0f, -1.0f, -1.0f) }); // 3: Front Bot Right
    m_Vertices.push_back({ XMFLOAT3(-1.0f,  1.0f,  1.0f) }); // 4: Back Top Left
    m_Vertices.push_back({ XMFLOAT3(1.0f,  1.0f,  1.0f) }); // 5: Back Top Right
    m_Vertices.push_back({ XMFLOAT3(-1.0f, -1.0f,  1.0f) }); // 6: Back Bot Left
    m_Vertices.push_back({ XMFLOAT3(1.0f, -1.0f,  1.0f) }); // 7: Back Bot Right

    // Helper lambda to generate inward-facing triangles given the 4 corners of a face
    // Corners must be passed in visual "reading order": TopLeft, TopRight, BotLeft, BotRight
    auto AddFaceIndices = [&](int tl, int tr, int bl, int br)
        {
            // Triangle 1
            m_Indices.push_back(tl);
            m_Indices.push_back(bl);
            m_Indices.push_back(tr);

            // Triangle 2
            m_Indices.push_back(bl);
            m_Indices.push_back(br);
            m_Indices.push_back(tr);
        };

    // +Z Face (Back)
    AddFaceIndices(5, 4, 7, 6);

    // -Z Face (Front)
    AddFaceIndices(0, 1, 2, 3);

    // +X Face (Right)
    AddFaceIndices(1, 5, 3, 7);

    // -X Face (Left)
    AddFaceIndices(4, 0, 6, 2);

    // +Y Face (Top)
    AddFaceIndices(4, 5, 0, 1);

    // -Y Face (Bottom)
    AddFaceIndices(2, 3, 6, 7);
}

ID3D11ShaderResourceView* const* SkyBox::GetPixelShaderSRVs()
{
    return m_SkyBoxTexture->GetTextureSRVs();
}