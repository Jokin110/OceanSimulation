#include "OceanSurface.h"
#include "CameraManager.h"
#include "OceanComputeManager.h"
#include "imgui.h"
#include <fstream>

#define MAX_OCEAN_PATCH_SIDE_VERTICES 512

OceanSurface::~OceanSurface()
{
	
}

bool OceanSurface::Initialize()
{
	bool result = Object::Initialize();

	m_Initialized = false;

	m_DomainShaderSRVCount = CASCADE_COUNT;
	m_PixelShaderSRVCount = CASCADE_COUNT;

	m_Initialized = true;

	return result;
}

void OceanSurface::Start()
{
	Object::Start();
}

void OceanSurface::Update()
{
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
	return OceanComputeManager::GetInstance().GetSlopeSRV();
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

	int numVertices = max(min(patchSize / vertexSeparation + 1, MAX_OCEAN_PATCH_SIDE_VERTICES), 2);

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

bool OceanSurface::RegenerateMeshAndPos(Vector3 position)
{
	ReleaseResources();

	bool value = Initialize();

	m_Position = position;

	return value;
}

void OceanSurface::UpdatePixelShaderBuffer(const PixelShaderConstantBufferData& pixelShaderBufferData)
{
	m_PixelShaderConstantBufferData = pixelShaderBufferData;

	if (GetPixelShaderConstantBuffers())
	{
		D3D11Application::GetInstance().GetDeviceContext()->UpdateSubresource(GetPixelShaderConstantBuffers(), 0, nullptr, &m_PixelShaderConstantBufferData, 0, 0);
	}
}