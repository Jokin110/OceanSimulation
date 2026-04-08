#include "OceanSurface.h"
#include "CameraManager.h"
#include "OceanComputeManager.h"
#include "imgui.h"
#include <fstream>

bool OceanSurface::Initialize()
{
	bool result = Object::Initialize();

	if (result)
	{
		m_PixelShaderBufferData = {};
		m_PixelShaderBufferData.m_FoamColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_PixelShaderBufferData.m_FoamBias = 0.3f;
		m_PixelShaderBufferData.m_LightColor = XMFLOAT3(0.53f, 0.81f, 0.92f);
		m_PixelShaderBufferData.m_AmbientLightIntensity = 0.25f;
		m_PixelShaderBufferData.m_LightDirection = XMFLOAT3(0.0f, -0.5f, -1.0f);
		m_PixelShaderBufferData.m_SpecularColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_PixelShaderBufferData.m_FogColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_PixelShaderBufferData.m_FogDistance = 800.0f;

		m_PixelShaderBufferData.m_UpwellingColor = XMFLOAT3(0.1f, 0.3f, 0.4f);
		m_PixelShaderBufferData.m_Snell = 1.33f;
		m_PixelShaderBufferData.m_AirColor = XMFLOAT3(0.1f, 0.1f, 0.1f);
		m_PixelShaderBufferData.m_kDiffuse = 0.01;

		D3D11Application::GetInstance().GetDeviceContext()->UpdateSubresource(GetPixelShaderBuffers(), 0, nullptr, &m_PixelShaderBufferData, 0, 0);
		m_UpdatePixelShaderBuffer = true;
	}

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

	ImGui::Begin("Ocean Surface Rendering Settings");
	ImGui::ColorEdit3("Foam Color", (float*)&m_PixelShaderBufferData.m_FoamColor);
	ImGui::SliderFloat("Foam Bias", &m_PixelShaderBufferData.m_FoamBias, 0.0f, 1.0f);
	ImGui::ColorEdit3("Light Color", (float*)&m_PixelShaderBufferData.m_LightColor);
	ImGui::SliderFloat("Ambient Light Intensity", &m_PixelShaderBufferData.m_AmbientLightIntensity, 0.0f, 1.0f);
	ImGui::SliderFloat3("Light Direction", (float*)&m_PixelShaderBufferData.m_LightDirection, -1.0f, 1.0f);
	ImGui::ColorEdit3("Specular Color", (float*)&m_PixelShaderBufferData.m_SpecularColor);
	ImGui::ColorEdit3("Fog Color", (float*)&m_PixelShaderBufferData.m_FogColor);
	ImGui::SliderFloat("Fog Distance", &m_PixelShaderBufferData.m_FogDistance, 0.0f, 2000.0f);
	ImGui::ColorEdit3("Upwelling Color", (float*)&m_PixelShaderBufferData.m_UpwellingColor);
	ImGui::SliderFloat("Snell's Index", &m_PixelShaderBufferData.m_Snell, 1.0f, 2.0f);
	ImGui::ColorEdit3("Air Color", (float*)&m_PixelShaderBufferData.m_AirColor);
	ImGui::SliderFloat("Diffuse Coefficient", &m_PixelShaderBufferData.m_kDiffuse, 0.0f, 1.0f);

	if (ImGui::Button("Apply Changes"))
	{
		D3D11Application::GetInstance().GetDeviceContext()->UpdateSubresource(GetPixelShaderBuffers(), 0, nullptr, &m_PixelShaderBufferData, 0, 0);
		m_UpdatePixelShaderBuffer = true;
	}

	if (ImGui::Button("Save Settings"))
	{
		std::ofstream outFile("OceanSettings.bin", std::ios::binary);
		if (outFile.is_open())
		{
			outFile.write(reinterpret_cast<const char*>(&m_PixelShaderBufferData), sizeof(m_PixelShaderBufferData));
			outFile.close();
		}
	}

	ImGui::SameLine();
	
	if (ImGui::Button("Load Settings"))
	{
		std::ifstream inFile("OceanSettings.bin", std::ios::binary);
		if (inFile.is_open())
		{
			inFile.read(reinterpret_cast<char*>(&m_PixelShaderBufferData), sizeof(m_PixelShaderBufferData));
			inFile.close();

			D3D11Application::GetInstance().GetDeviceContext()->UpdateSubresource(GetPixelShaderBuffers(), 0, nullptr, &m_PixelShaderBufferData, 0, 0);
			m_UpdatePixelShaderBuffer = true;
		}
	}

	ImGui::End();

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

	int widthVertices = textureSize;
	int depthVertices = textureSize;

	float separation = patchSize / (float) textureSize;

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