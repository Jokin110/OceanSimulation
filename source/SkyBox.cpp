#include "SkyBox.h"

bool SkyBox::Initialize()
{
	return Object::Initialize();
}

void SkyBox::Start()
{
	Object::Start();

    m_Scale = Vector3(1000, 1000, 1000);
}

void SkyBox::Update()
{
	XMMATRIX scaleMatrix = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_Rotation.x), XMConvertToRadians(m_Rotation.y), XMConvertToRadians(m_Rotation.z));
	XMMATRIX translationMatrix = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

	XMMATRIX worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	m_ConstantBufferData = {};
	m_ConstantBufferData.m_WorldViewProjectioMatrix = worldMatrix;

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
			offsetof(VertexDataSkyBox, position),
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"TEXCOORD0",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			offsetof(VertexDataSkyBox, texcoord),
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
    XMFLOAT2 uvTopLeft = XMFLOAT2(0.0f, 0.0f);
    XMFLOAT2 uvTopRight = XMFLOAT2(1.0f, 0.0f);
    XMFLOAT2 uvBotLeft = XMFLOAT2(0.0f, 1.0f);
    XMFLOAT2 uvBotRight = XMFLOAT2(1.0f, 1.0f);

    // Helper to push a face (4 vertices) and 2 triangles (6 indices)
    // We pass vertices in standard "reading order" (TopLeft, TopRight, BotLeft, BotRight)
    // But we generate indices to face INWARDS.
    auto AddFace = [&](XMFLOAT3 tl, XMFLOAT3 tr, XMFLOAT3 bl, XMFLOAT3 br)
        {
            // Current index offset
            int startIdx = static_cast<int>(m_Vertices.size());

            // Add 4 vertices for this face
            m_Vertices.push_back({ tl, uvTopLeft });  // 0: Top-Left
            m_Vertices.push_back({ tr, uvTopRight }); // 1: Top-Right
            m_Vertices.push_back({ bl, uvBotLeft });  // 2: Bottom-Left
            m_Vertices.push_back({ br, uvBotRight }); // 3: Bottom-Right

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
    AddFace(p5, p4, p7, p6);

    // -Z Face (Front)
    AddFace(p0, p1, p2, p3);

    // +X Face (Right)
    AddFace(p1, p5, p3, p7);

    // -X Face (Left)
    AddFace(p4, p0, p6, p2);

    // +Y Face (Top)
    AddFace(p4, p5, p0, p1);

    // -Y Face (Bottom)
    AddFace(p2, p3, p6, p7);
}