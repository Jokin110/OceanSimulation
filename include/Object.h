#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "Vector3.h"
#include "TimeManager.h"
#include "D3D11Application.h"
#include "ObjectManager.h"
#include <string>

using namespace DirectX;
using namespace std;

class IObject
{
public:
	virtual ~IObject() = default;

	virtual bool Initialize() = 0;
	virtual void Start() = 0;
	virtual void Update() = 0;

	virtual ID3D11InputLayout*& GetInputLayout() = 0;
	virtual ID3D11Buffer*& GetVertexBuffer() = 0;
	virtual ID3D11Buffer*& GetIndexBuffer() = 0;

	virtual ID3D11VertexShader*& GetVertexShader() = 0;
	virtual ID3D11PixelShader*& GetPixelShader() = 0; 
	
	virtual ID3D11HullShader*& GetHullShader() = 0;
	virtual ID3D11DomainShader*& GetDomainShader() = 0;
	virtual D3D11_PRIMITIVE_TOPOLOGY GetTopology() = 0;

	virtual ID3D11SamplerState*& GetSamplerState() = 0;

	virtual ID3D11Buffer*& GetConstantBuffers() = 0;
	virtual ID3D11Buffer*& GetPixelShaderBuffers() = 0;

	virtual bool GetUpdatePixelShaderBuffer() = 0;
	virtual void SetUpdatePixelShaderBuffer(bool update) = 0;

	virtual UINT GetIndexCount() = 0;

	virtual UINT GetVertexStride() = 0;

	virtual bool UseTessellation() = 0;

	virtual Vector3 GetPosition() = 0;
	virtual void SetPosition(const Vector3& position) = 0;
	virtual Vector3 GetRotation() = 0;
	virtual void SetRotation(const Vector3& rotation) = 0;
	virtual Vector3 GetScale() = 0;
	virtual void SetScale(const Vector3& scale) = 0;
};

template<typename VertexData, typename ConstantBufferData, typename PixelShaderBufferData>
class Object : IObject
{
public:
	Object();
	Object(string name);
	Object(string name, wstring vertexShaderFile, wstring pixelShaderFile);
	Object(string name, wstring hullShaderFile, wstring domainShaderFile, D3D11_PRIMITIVE_TOPOLOGY topology);
	Object(string name, wstring vertexShaderFile, wstring pixelShaderFile, wstring hullShaderFile, wstring domainShaderFile, D3D11_PRIMITIVE_TOPOLOGY topology);
	~Object();

	virtual bool Initialize();
	virtual void Start();
	virtual void Update();

	ID3D11InputLayout*& GetInputLayout() { return m_d3dInputLayout; }
	ID3D11Buffer*& GetVertexBuffer() { return m_d3dVertexBuffer; }
	ID3D11Buffer*& GetIndexBuffer() { return m_d3dIndexBuffer; }

	ID3D11VertexShader*& GetVertexShader() { return m_d3dVertexShader; }
	ID3D11PixelShader*& GetPixelShader() { return m_d3dPixelShader; }

	ID3D11HullShader*& GetHullShader() { return m_d3dHullShader; }
	ID3D11DomainShader*& GetDomainShader() { return m_d3dDomainShader; }
	D3D11_PRIMITIVE_TOPOLOGY GetTopology() { return m_Topology; }

	ID3D11SamplerState*& GetSamplerState() { return m_d3dSamplerState; }

	ID3D11Buffer*& GetConstantBuffers() { return m_d3dConstantBuffers; }
	ID3D11Buffer*& GetPixelShaderBuffers() { return m_d3dPixelShaderBuffers; }

	bool GetUpdatePixelShaderBuffer() { return m_UpdatePixelShaderBuffer; }
	void SetUpdatePixelShaderBuffer(bool update) { m_UpdatePixelShaderBuffer = update; }

	UINT GetIndexCount() { return static_cast<UINT>(m_Indices.size()); }

	UINT GetVertexStride() { return sizeof(VertexData); }

	bool UseTessellation() { return !m_HullShaderFile.empty() && !m_DomainShaderFile.empty(); }

	Vector3 GetPosition() { return m_Position; }
	void SetPosition(const Vector3& position) { m_Position = position; }
	Vector3 GetRotation() { return m_Rotation; }
	void SetRotation(const Vector3& rotation) { m_Rotation = rotation; }
	Vector3 GetScale() { return m_Scale; }
	void SetScale(const Vector3& scale) { m_Scale = scale; }

protected:
	string m_Name;
	Vector3 m_Position;
	Vector3 m_Rotation;
	Vector3 m_Scale;

	wstring m_VertexShaderFile;
	wstring m_PixelShaderFile;

	wstring m_HullShaderFile = L"";
	wstring m_DomainShaderFile = L"";
	D3D11_PRIMITIVE_TOPOLOGY m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	virtual UINT GetVertexInputLayout(D3D11_INPUT_ELEMENT_DESC*& inputLayout) = 0;
	void GetConstantBufferDesc(D3D11_BUFFER_DESC& constantBufferDesc);
	void GetPixelShaderBufferDesc(D3D11_BUFFER_DESC& pixelShaderBufferDesc);

	vector<VertexData> m_Vertices;
	vector<int> m_Indices;

	ID3D11SamplerState* m_d3dSamplerState = nullptr;

	ConstantBufferData m_ConstantBufferData;
	PixelShaderBufferData m_PixelShaderBufferData;

	bool m_UpdatePixelShaderBuffer = false;

	virtual void GenerateMesh() = 0;

private:
	ID3D11InputLayout* m_d3dInputLayout = nullptr;
	ID3D11Buffer* m_d3dVertexBuffer = nullptr;
	ID3D11Buffer* m_d3dIndexBuffer = nullptr;

	ID3D11VertexShader* m_d3dVertexShader = nullptr;
	ID3D11PixelShader* m_d3dPixelShader = nullptr; 
	
	ID3D11HullShader* m_d3dHullShader = nullptr;
	ID3D11DomainShader* m_d3dDomainShader = nullptr;

	ID3D11Buffer* m_d3dConstantBuffers = nullptr;
	ID3D11Buffer* m_d3dPixelShaderBuffers = nullptr;

	UINT GetVerticesByteWidth() { return static_cast<UINT>(sizeof(VertexData) * m_Vertices.size()); }
	UINT GetIndicesByteWidth() { return static_cast<UINT>(sizeof(int) * m_Indices.size()); }

	template<typename T>
	inline void SafeRelease(T& ptr)
	{
		if (ptr != NULL)
		{
			ptr->Release();
			ptr = NULL;
		}
	}
};

template<typename VertexData, typename ConstantBufferData, typename PixelShaderBufferData>
Object<VertexData, ConstantBufferData, PixelShaderBufferData>::Object()
{
	m_Name = "New Object";

	m_Position = Vector3(0.0f, 0.0f, 0.0f);
	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
	m_Scale = Vector3(1.0f, 1.0f, 1.0f);

	m_VertexShaderFile = L"assets/shaders/GerstnerWavesVS.hlsl";
	m_PixelShaderFile = L"assets/shaders/PixelShader.hlsl";

	m_d3dInputLayout = nullptr;
	m_d3dVertexBuffer = nullptr;
	m_d3dIndexBuffer = nullptr;
	m_d3dVertexShader = nullptr;
	m_d3dPixelShader = nullptr;
	m_d3dHullShader = nullptr;
	m_d3dDomainShader = nullptr;
	m_d3dConstantBuffers = nullptr;
	m_d3dPixelShaderBuffers = nullptr;

	m_d3dSamplerState = nullptr;

	m_ConstantBufferData = {};
	m_PixelShaderBufferData = {};

	ObjectManager::GetInstance().AddObjectToList(this);
}

template<typename VertexData, typename ConstantBufferData, typename PixelShaderBufferData>
Object<VertexData, ConstantBufferData, PixelShaderBufferData>::Object(string name) : Object()
{
	m_Name = name;
}

template<typename VertexData, typename ConstantBufferData, typename PixelShaderBufferData>
Object<VertexData, ConstantBufferData, PixelShaderBufferData>::Object(string name, wstring vertexShaderFile, wstring pixelShaderFile) : Object(name)
{
	m_VertexShaderFile = vertexShaderFile;
	m_PixelShaderFile = pixelShaderFile;
}

template<typename VertexData, typename ConstantBufferData, typename PixelShaderBufferData>
Object<VertexData, ConstantBufferData, PixelShaderBufferData>::Object(string name, wstring hullShaderFile, wstring domainShaderFile, D3D11_PRIMITIVE_TOPOLOGY topology) : Object(name)
{
	m_HullShaderFile = hullShaderFile;
	m_DomainShaderFile = domainShaderFile;
	m_Topology = topology;
}

template<typename VertexData, typename ConstantBufferData, typename PixelShaderBufferData>
Object<VertexData, ConstantBufferData, PixelShaderBufferData>::Object(string name, wstring vertexShaderFile, wstring pixelShaderFile, wstring hullShaderFile, wstring domainShaderFile, D3D11_PRIMITIVE_TOPOLOGY topology) : Object(name)
{
	m_VertexShaderFile = vertexShaderFile;
	m_PixelShaderFile = pixelShaderFile;
	m_HullShaderFile = hullShaderFile;
	m_DomainShaderFile = domainShaderFile;
	m_Topology = topology;
}

template<typename VertexData, typename ConstantBufferData, typename PixelShaderBufferData>
Object<VertexData, ConstantBufferData, PixelShaderBufferData>::~Object()
{
	SafeRelease(m_d3dInputLayout);
	SafeRelease(m_d3dVertexBuffer);
	SafeRelease(m_d3dIndexBuffer);
	SafeRelease(m_d3dVertexShader);
	SafeRelease(m_d3dPixelShader);
	SafeRelease(m_d3dHullShader);
	SafeRelease(m_d3dDomainShader);
	SafeRelease(m_d3dConstantBuffers);

	SafeRelease(m_d3dSamplerState);

	m_Vertices.clear();
	m_Indices.clear();
}

template<typename VertexData, typename ConstantBufferData, typename PixelShaderBufferData>
bool Object<VertexData, ConstantBufferData, PixelShaderBufferData>::Initialize()
{
	GenerateMesh();

	ID3DBlob* vertexShaderBlob = nullptr;
#if _DEBUG
	m_d3dVertexShader = D3D11Application::GetInstance().CreateVertexShader(m_VertexShaderFile, vertexShaderBlob);
#else
	m_d3dVertexShader = D3D11Application::GetInstance().CreateVertexShader(L"../../" + m_VertexShaderFile, vertexShaderBlob);
#endif

	if (m_d3dVertexShader == nullptr)
	{
		return false;
	}

#if _DEBUG
	m_d3dPixelShader = D3D11Application::GetInstance().CreatePixelShader(m_PixelShaderFile);
#else
	m_d3dPixelShader = D3D11Application::GetInstance().CreatePixelShader(L"../../" + m_PixelShaderFile);
#endif

	if (m_d3dPixelShader == nullptr)
	{
		return false;
	}

	if (!m_HullShaderFile.empty())
	{
#if _DEBUG
		m_d3dHullShader = D3D11Application::GetInstance().CreateHullShader(m_HullShaderFile);
#else
		m_d3dHullShader = D3D11Application::GetInstance().CreateHullShader(L"../../" + m_HullShaderFile);
#endif
		if (m_d3dHullShader == nullptr)
		{
			return false;
		}
	}

	if (!m_DomainShaderFile.empty())
	{
#if _DEBUG
		m_d3dDomainShader = D3D11Application::GetInstance().CreateDomainShader(m_DomainShaderFile);
#else
		m_d3dDomainShader = D3D11Application::GetInstance().CreateDomainShader(L"../../" + m_DomainShaderFile);
#endif
		if (m_d3dDomainShader == nullptr)
		{
			return false;
		}
	}

	D3D11_INPUT_ELEMENT_DESC* vertexInputLayout = nullptr;
	UINT numElements = GetVertexInputLayout(vertexInputLayout);

	if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateInputLayout(
		vertexInputLayout,
		numElements,
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize(),
		&m_d3dInputLayout)))
	{
		cout << "D3D11: Failed to create default vertex input layout\n";
		return false;
	}

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.ByteWidth = GetVerticesByteWidth();
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA resourceData = {};
	resourceData.pSysMem = m_Vertices.data();

	if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateBuffer(
		&vertexBufferDesc,
		&resourceData,
		&m_d3dVertexBuffer)))
	{
		cout << "D3D11: Failed to create triangle vertex buffer\n";
		return false;
	}

	D3D11_BUFFER_DESC indexBufferDesc = {};

	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.ByteWidth = GetIndicesByteWidth();
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	resourceData.pSysMem = m_Indices.data();

	if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateBuffer(
		&indexBufferDesc,
		&resourceData,
		&m_d3dIndexBuffer)))
	{
		cout << "D3D11: Failed to create triangle index buffer\n";
		return false;
	}

	D3D11_BUFFER_DESC constantBufferDesc = {};
	GetConstantBufferDesc(constantBufferDesc);

	if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateBuffer(
		&constantBufferDesc,
		nullptr,
		&m_d3dConstantBuffers)))
	{
		cout << "D3D11: Failed to create per-object constant buffer\n";
		return false;
	}

	D3D11_BUFFER_DESC pixelShaderBufferDesc = {};
	GetConstantBufferDesc(pixelShaderBufferDesc);

	if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateBuffer(
		&pixelShaderBufferDesc,
		nullptr,
		&m_d3dPixelShaderBuffers)))
	{
		cout << "D3D11: Failed to create per-object pixel shader buffer\n";
		return false;
	}

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
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

	return true;
}

template<typename VertexData, typename ConstantBufferData, typename PixelShaderBufferData>
void Object<VertexData, ConstantBufferData, PixelShaderBufferData>::Start()
{

}

template<typename VertexData, typename ConstantBufferData, typename PixelShaderBufferData>
void Object<VertexData, ConstantBufferData, PixelShaderBufferData>::Update()
{
	D3D11Application::GetInstance().GetDeviceContext()->UpdateSubresource(m_d3dConstantBuffers, 0, nullptr, &m_ConstantBufferData, 0, 0);
}

template<typename VertexData, typename ConstantBufferData, typename PixelShaderBufferData>
void Object<VertexData, ConstantBufferData, PixelShaderBufferData>::GetConstantBufferDesc(D3D11_BUFFER_DESC& constantBufferDesc)
{
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(ConstantBufferData);
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
}

template<typename VertexData, typename ConstantBufferData, typename PixelShaderBufferData>
void Object<VertexData, ConstantBufferData, PixelShaderBufferData>::GetPixelShaderBufferDesc(D3D11_BUFFER_DESC& constantBufferDesc)
{
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(PixelShaderBufferData);
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
}