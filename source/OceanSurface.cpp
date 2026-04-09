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

	m_ConstantBufferData = {};
	m_ConstantBufferData.m_WorldMatrix = worldMatrix;
	m_ConstantBufferData.m_InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));
	m_ConstantBufferData.m_ViewProjectionMatrix = XMMatrixMultiply(CameraManager::GetInstance().GetViewMatrix(), CameraManager::GetInstance().GetProjectionMatrix());
	m_ConstantBufferData.m_Time = TimeManager::GetInstance().GetTime();
	m_ConstantBufferData.m_CameraPosition = CameraManager::GetInstance().GetCameraPosition();
	m_ConstantBufferData.m_OceanTextureSize = OceanComputeManager::GetInstance().GetOceanTextureSize();
	m_ConstantBufferData.m_PatchSize = static_cast<float>(OceanComputeManager::GetInstance().GetOceanPatchSize());

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
	float patchSize = OceanComputeManager::GetInstance().GetOceanPatchSize();

	int numSubdivisions = 64;

	int widthVertices = textureSize / numSubdivisions + 1;
	int depthVertices = textureSize / numSubdivisions + 1;

	float separation = patchSize / (float)(textureSize / numSubdivisions);

	float startX = -((widthVertices - 1) * separation) / 2.0f;
	float startZ = -((depthVertices - 1) * separation) / 2.0f;

	for (int x = 0; x < widthVertices; x++)
	{
		for (int z = 0; z < depthVertices; z++)
		{
			VertexData vertex = VertexData{};

			vertex.position.x = startX + x * separation;
			vertex.position.y = 0.0f;
			vertex.position.z = startZ + z * separation;

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
			m_Indices.push_back(bottomRightVertex);
			m_Indices.push_back(topRightVertex);
		}
	}
}

void OceanSurface::UpdatePixelShaderBuffer(const PerObjectPixelShaderBufferData& pixelShaderBufferData)
{
	m_PixelShaderBufferData = pixelShaderBufferData;
	D3D11Application::GetInstance().GetDeviceContext()->UpdateSubresource(GetPixelShaderBuffers(), 0, nullptr, &m_PixelShaderBufferData, 0, 0);
	m_UpdatePixelShaderBuffer = true;
}