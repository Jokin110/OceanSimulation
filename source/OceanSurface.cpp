#include "OceanSurface.h"
#include "CameraManager.h"
#include "OceanComputeManager.h"
#include "imgui.h"
#include <fstream>

bool OceanSurface::Initialize()
{
	bool result = Object::Initialize();

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

UINT OceanSurface::GetVertexInputLayout(D3D11_INPUT_ELEMENT_DESC*& inputLayout)
{
	static D3D11_INPUT_ELEMENT_DESC vertexInputLayout[] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(VertexData, position),
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(VertexData, color),
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
	float patchSize = OceanComputeManager::GetInstance().GetOceanPatchSize()[0];

	float vertexSeparation = OceanComputeManager::GetInstance().GetMeshVertexSeparation();

	int numVertices = patchSize / vertexSeparation + 1;

	float moduleSeparation = (((float) patchSize / vertexSeparation) - numVertices + 1) * vertexSeparation;

	if (moduleSeparation > 0.0f)
	{
		numVertices++;
	}

	float startX = -patchSize / 2.0f;
	float startZ = -patchSize / 2.0f;

	for (int x = 0; x < numVertices; x++)
	{
		for (int z = 0; z < numVertices; z++)
		{
			float xSeparation = vertexSeparation;
			float zSeparation = vertexSeparation;

			if (moduleSeparation > 0.0f)
			{
				if (x == numVertices - 1) xSeparation = moduleSeparation;
				if (z == numVertices - 1 ) zSeparation = moduleSeparation;
			}

			VertexData vertex = VertexData{};

			vertex.position.x = startX + (x - 1) * vertexSeparation + xSeparation;
			vertex.position.y = 0.0f;
			vertex.position.z = startZ + (z - 1) * vertexSeparation + zSeparation;

			vertex.color = XMFLOAT3(0.0f, 0.41f, 0.58f);

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

void OceanSurface::RegenerateMeshAndPos(Vector3 position)
{
	Initialize();

	m_Position = position;
}

void OceanSurface::UpdatePixelShaderBuffer(const PixelShaderConstantBufferData& pixelShaderBufferData)
{
	m_PixelShaderConstantBufferData = pixelShaderBufferData;
	D3D11Application::GetInstance().GetDeviceContext()->UpdateSubresource(GetPixelShaderConstantBuffers(), 0, nullptr, &m_PixelShaderConstantBufferData, 0, 0);
	m_UpdatePixelShaderBuffer = true;
}