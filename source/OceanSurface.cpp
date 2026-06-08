#include "OceanSurface.h"
#include "CameraManager.h"
#include "OceanComputeManager.h"
#include "SceneManager.h"
#include "imgui.h"
#include <fstream>

#define MAX_OCEAN_PATCH_SIDE_VERTICES 512

OceanSurface::OceanSurface(string name, wstring vertexShaderFile, wstring pixelShaderFile, wstring hullShaderFile, wstring domainShaderFile, D3D11_PRIMITIVE_TOPOLOGY topology) : Object(name, vertexShaderFile, pixelShaderFile, hullShaderFile, domainShaderFile, topology)
{
	m_FoamTexture = new Texture2D(1, &m_FoamTextureFilePath, false, false, true, false);

	m_PixelShaderSRVs = nullptr;
}

OceanSurface::~OceanSurface()
{
	ReleaseResources();
}

bool OceanSurface::Initialize()
{
	bool result = Object::Initialize();

	m_Initialized = false;

	result = result && m_FoamTexture->Initialize();

	if (m_d3dSamplerState)
	{
		m_d3dSamplerState->Release();
		m_d3dSamplerState = nullptr;
	}

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 16;
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

	m_DomainShaderSRVCount = CASCADE_COUNT;
	m_PixelShaderSRVCount = 2 * CASCADE_COUNT + 2;

	m_PixelShaderSRVs = new ID3D11ShaderResourceView * [m_PixelShaderSRVCount];

	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		m_PixelShaderSRVs[i] = OceanComputeManager::GetInstance().GetSlopeSRV()[i];
	}

	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		m_PixelShaderSRVs[i + CASCADE_COUNT] = OceanComputeManager::GetInstance().GetSecondOrderMomentsSRV()[i];
	}

	m_PixelShaderSRVs[2 * CASCADE_COUNT] = SceneManager::GetInstance().GetSkyboxSRV()[0];

	m_PixelShaderSRVs[2 * CASCADE_COUNT + 1] = m_FoamTexture->GetTextureSRVs()[0];

	m_Initialized = true;

	return result;
}

void OceanSurface::Start()
{
	Object::Start();

	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		m_PixelShaderSRVs[i] = OceanComputeManager::GetInstance().GetSlopeSRV()[i];
	}

	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		m_PixelShaderSRVs[i + CASCADE_COUNT] = OceanComputeManager::GetInstance().GetSecondOrderMomentsSRV()[i];
	}

	m_PixelShaderSRVs[2 * CASCADE_COUNT] = SceneManager::GetInstance().GetSkyboxSRV()[0];
}

void OceanSurface::Update()
{
	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		m_PixelShaderSRVs[i] = OceanComputeManager::GetInstance().GetSlopeSRV()[i];
	}

	XMMATRIX scaleMatrix = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_Rotation.x), XMConvertToRadians(m_Rotation.y), XMConvertToRadians(m_Rotation.z));
	XMMATRIX translationMatrix = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

	XMMATRIX worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	m_HullShaderConstantBufferData = {};
	m_HullShaderConstantBufferData.m_WorldMatrix = worldMatrix;
	m_HullShaderConstantBufferData.m_CameraPosition = CameraManager::GetInstance().GetCameraPosition();

	TessellationSettingsData tessSettings = OceanComputeManager::GetInstance().GetTessellationSettingsData();
	m_HullShaderConstantBufferData.m_MinDistance = tessSettings.m_MinTessellationDistance;
	m_HullShaderConstantBufferData.m_MaxDistance = tessSettings.m_MaxTessellationDistance;
	m_HullShaderConstantBufferData.m_TessFactorExponent = tessSettings.m_TessellationExponent;

	m_DomainShaderConstantBufferData = {};
	m_DomainShaderConstantBufferData.m_WorldMatrix = worldMatrix;
	m_DomainShaderConstantBufferData.m_ViewProjectionMatrix = XMMatrixMultiply(CameraManager::GetInstance().GetViewMatrix(), CameraManager::GetInstance().GetProjectionMatrix());
	m_DomainShaderConstantBufferData.m_CameraPosition = CameraManager::GetInstance().GetCameraPosition();

	const float* patches = OceanComputeManager::GetInstance().GetOceanPatchSize();
	m_DomainShaderConstantBufferData.m_PatchSizes = XMFLOAT4(patches[0], patches[1], patches[2], patches[3]);

	Object::Update();
}

ID3D11ShaderResourceView* const* OceanSurface::GetDomainShaderSRVs()
{
	return OceanComputeManager::GetInstance().GetDisplacementSRV();
}

ID3D11ShaderResourceView* const* OceanSurface::GetPixelShaderSRVs()
{
	return m_PixelShaderSRVs;
}

UINT OceanSurface::GetVertexInputLayout(D3D11_INPUT_ELEMENT_DESC*& inputLayout)
{
	static D3D11_INPUT_ELEMENT_DESC vertexInputLayout[] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(VertexData, m_Position),
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(VertexData, m_Color),
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};

	inputLayout = vertexInputLayout;

	return _countof(vertexInputLayout);
}

void OceanSurface::GenerateMesh()
{
	m_Vertices.clear();
	m_Indices.clear();

	int textureSize = OceanComputeManager::GetInstance().GetOceanTextureSize();
	float patchSize = OceanComputeManager::GetInstance().GetOceanMeshPatchSize();

	float vertexSeparation = max(OceanComputeManager::GetInstance().GetMeshVertexSeparation(), 0.1f);

	int numVertices = static_cast<int> max(min(patchSize / vertexSeparation + 1, MAX_OCEAN_PATCH_SIDE_VERTICES), 2);

	vertexSeparation = patchSize / (numVertices - 1);

	float startX = -patchSize / 2.0f;
	float startZ = -patchSize / 2.0f;

	for (int x = 0; x < numVertices; x++)
	{
		for (int z = 0; z < numVertices; z++)
		{
			VertexData vertex = VertexData{};

			vertex.m_Position.x = startX + x * vertexSeparation;
			vertex.m_Position.y = 0.0f;
			vertex.m_Position.z = startZ + z * vertexSeparation;

			vertex.m_Color = XMFLOAT3(0.0f, 0.41f, 0.58f);

			m_Vertices.push_back(vertex);
		}
	}

	for (int x = 0; x < numVertices - 1; x++)
	{
		for (int z = 0; z < numVertices - 1; z++)
		{
			int bottomLeftVertex = x * numVertices + z;
			int bottomRightVertex = (x + 1) * numVertices + z;
			int topLeftVertex = bottomLeftVertex + 1;
			int topRightVertex = bottomRightVertex + 1;

			m_Indices.push_back(bottomLeftVertex);
			m_Indices.push_back(topLeftVertex);
			m_Indices.push_back(bottomRightVertex);
			m_Indices.push_back(topRightVertex);
		}
	}
}

void OceanSurface::ReleaseResources()
{
	Object::ReleaseResources();

	if (m_FoamTexture)
	{
		delete m_FoamTexture;
		m_FoamTexture = nullptr;
	}

	if (m_PixelShaderSRVs) { delete[] m_PixelShaderSRVs; m_PixelShaderSRVs = nullptr; };
}

bool OceanSurface::RegenerateMeshAndPos(Vector3 position)
{
	ReleaseResources();

	m_FoamTexture = new Texture2D(1, &m_FoamTextureFilePath, false, false, true, false);

	bool value = Initialize();

	m_Position = position;

	return value;
}

void OceanSurface::UpdatePixelShaderBuffer(const PixelShaderConstantBufferData& pixelShaderBufferData)
{
	m_PixelShaderConstantBufferData = pixelShaderBufferData;

	/*if (GetPixelShaderConstantBuffers())
	{
		D3D11Application::GetInstance().GetDeviceContext()->UpdateSubresource(GetPixelShaderConstantBuffers(), 0, nullptr, &m_PixelShaderConstantBufferData, 0, 0);
	}*/
}