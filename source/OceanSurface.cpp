#include "OceanSurface.h"
#include "CameraManager.h"

bool OceanSurface::Initialize()
{
	return Object::Initialize();
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
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(VertexData, normal),
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

	int widthVertices = 1024;
	int depthVertices = 1024;

	float separation = 0.1f;

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