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

	virtual bool IsInitialized() = 0;

	virtual void ReleaseResources() = 0;

	virtual ID3D11InputLayout*& GetInputLayout() = 0;
	virtual ID3D11Buffer*& GetVertexBuffer() = 0;
	virtual ID3D11Buffer*& GetIndexBuffer() = 0;

	virtual ID3D11VertexShader*& GetVertexShader() = 0;
	virtual ID3D11PixelShader*& GetPixelShader() = 0; 
	
	virtual ID3D11HullShader*& GetHullShader() = 0;
	virtual ID3D11DomainShader*& GetDomainShader() = 0;
	virtual D3D11_PRIMITIVE_TOPOLOGY GetTopology() = 0;

	virtual ID3D11SamplerState*& GetSamplerState() = 0;

	virtual ID3D11Buffer*& GetVertexShaderConstantBuffers() = 0;
	virtual ID3D11Buffer*& GetPixelShaderConstantBuffers() = 0;
	virtual ID3D11Buffer*& GetHullShaderConstantBuffers() = 0;
	virtual ID3D11Buffer*& GetDomainShaderConstantBuffers() = 0;

	virtual ID3D11ShaderResourceView* const* GetVertexShaderSRVs() = 0;
	virtual ID3D11ShaderResourceView* const* GetPixelShaderSRVs() = 0;
	virtual ID3D11ShaderResourceView* const* GetHullShaderSRVs() = 0;
	virtual ID3D11ShaderResourceView* const* GetDomainShaderSRVs() = 0;

	virtual UINT GetVertexShaderSRVCount() = 0;
	virtual UINT GetPixelShaderSRVCount() = 0;
	virtual UINT GetHullShaderSRVCount() = 0;
	virtual UINT GetDomainShaderSRVCount() = 0;

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

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
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

	bool IsInitialized() { return m_Initialized; }

	ID3D11InputLayout*& GetInputLayout() { return m_d3dInputLayout; }
	ID3D11Buffer*& GetVertexBuffer() { return m_d3dVertexBuffer; }
	ID3D11Buffer*& GetIndexBuffer() { return m_d3dIndexBuffer; }

	ID3D11VertexShader*& GetVertexShader() { return m_d3dVertexShader; }
	ID3D11PixelShader*& GetPixelShader() { return m_d3dPixelShader; }

	ID3D11HullShader*& GetHullShader() { return m_d3dHullShader; }
	ID3D11DomainShader*& GetDomainShader() { return m_d3dDomainShader; }
	D3D11_PRIMITIVE_TOPOLOGY GetTopology() { return m_Topology; }

	ID3D11SamplerState*& GetSamplerState() { return m_d3dSamplerState; }

	ID3D11Buffer*& GetVertexShaderConstantBuffers() { return m_d3dVertexShaderConstantBuffers; }
	ID3D11Buffer*& GetPixelShaderConstantBuffers() { return m_d3dPixelShaderConstantBuffers; }
	ID3D11Buffer*& GetHullShaderConstantBuffers() { return m_d3dHullShaderConstantBuffers; }
	ID3D11Buffer*& GetDomainShaderConstantBuffers() { return m_d3dDomainShaderConstantBuffers; }

	virtual ID3D11ShaderResourceView* const* GetVertexShaderSRVs();
	virtual ID3D11ShaderResourceView* const* GetPixelShaderSRVs();
	virtual ID3D11ShaderResourceView* const* GetHullShaderSRVs();
	virtual ID3D11ShaderResourceView* const* GetDomainShaderSRVs();

	UINT GetVertexShaderSRVCount() { return m_VertexShaderSRVCount; }
	UINT GetPixelShaderSRVCount() { return m_PixelShaderSRVCount; }
	UINT GetHullShaderSRVCount() { return m_HullShaderSRVCount; }
	UINT GetDomainShaderSRVCount() { return m_DomainShaderSRVCount; }

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
	bool m_Initialized;

	string m_Name;
	Vector3 m_Position;
	Vector3 m_Rotation;
	Vector3 m_Scale;

	wstring m_VertexShaderFile;
	wstring m_PixelShaderFile;

	wstring m_HullShaderFile = L"";
	wstring m_DomainShaderFile = L"";
	D3D11_PRIMITIVE_TOPOLOGY m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	void ReleaseResources();

	virtual UINT GetVertexInputLayout(D3D11_INPUT_ELEMENT_DESC*& inputLayout) = 0;
	void GetVertexShaderConstantBufferDesc(D3D11_BUFFER_DESC& constantBufferDesc);
	void GetPixelShaderConstantBufferDesc(D3D11_BUFFER_DESC& pixelShaderBufferDesc);
	void GetHullShaderConstantBufferDesc(D3D11_BUFFER_DESC& hullShaderBufferDesc);
	void GetDomainShaderConstantBufferDesc(D3D11_BUFFER_DESC& domainShaderBufferDesc);

	vector<VertexData> m_Vertices;
	vector<int> m_Indices;

	ID3D11SamplerState* m_d3dSamplerState = nullptr;

	VertexShaderConstantBufferData m_VertexShaderConstantBufferData;
	PixelShaderConstantBufferData m_PixelShaderConstantBufferData;
	HullShaderConstantBufferData m_HullShaderConstantBufferData;
	DomainShaderConstantBufferData m_DomainShaderConstantBufferData;

	UINT m_VertexShaderSRVCount = 0;
	UINT m_PixelShaderSRVCount = 0;
	UINT m_HullShaderSRVCount = 0;
	UINT m_DomainShaderSRVCount = 0;

	virtual void GenerateMesh() = 0;

private:
	ID3D11InputLayout* m_d3dInputLayout = nullptr;
	ID3D11Buffer* m_d3dVertexBuffer = nullptr;
	ID3D11Buffer* m_d3dIndexBuffer = nullptr;

	ID3D11VertexShader* m_d3dVertexShader = nullptr;
	ID3D11PixelShader* m_d3dPixelShader = nullptr; 
	
	ID3D11HullShader* m_d3dHullShader = nullptr;
	ID3D11DomainShader* m_d3dDomainShader = nullptr;

	ID3D11Buffer* m_d3dVertexShaderConstantBuffers = nullptr;
	ID3D11Buffer* m_d3dPixelShaderConstantBuffers = nullptr;
	ID3D11Buffer* m_d3dHullShaderConstantBuffers = nullptr;
	ID3D11Buffer* m_d3dDomainShaderConstantBuffers = nullptr;

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

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::Object()
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
	m_d3dVertexShaderConstantBuffers = nullptr;
	m_d3dPixelShaderConstantBuffers = nullptr;
	m_d3dHullShaderConstantBuffers = nullptr;
	m_d3dDomainShaderConstantBuffers = nullptr;

	m_d3dSamplerState = nullptr;

	m_VertexShaderConstantBufferData = {};
	m_PixelShaderConstantBufferData = {};
	m_HullShaderConstantBufferData = {};
	m_DomainShaderConstantBufferData = {};

	ObjectManager::GetInstance().AddObjectToList(this);
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::Object(string name) : Object()
{
	m_Name = name;
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::Object(string name, wstring vertexShaderFile, wstring pixelShaderFile) : Object(name)
{
	m_VertexShaderFile = vertexShaderFile;
	m_PixelShaderFile = pixelShaderFile;
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::Object(string name, wstring hullShaderFile, wstring domainShaderFile, D3D11_PRIMITIVE_TOPOLOGY topology) : Object(name)
{
	m_HullShaderFile = hullShaderFile;
	m_DomainShaderFile = domainShaderFile;
	m_Topology = topology;
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::Object(string name, wstring vertexShaderFile, wstring pixelShaderFile, wstring hullShaderFile, wstring domainShaderFile, D3D11_PRIMITIVE_TOPOLOGY topology) : Object(name)
{
	m_VertexShaderFile = vertexShaderFile;
	m_PixelShaderFile = pixelShaderFile;
	m_HullShaderFile = hullShaderFile;
	m_DomainShaderFile = domainShaderFile;
	m_Topology = topology;
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::~Object()
{
	ReleaseResources();
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
void Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::ReleaseResources()
{
	SafeRelease(m_d3dInputLayout);
	SafeRelease(m_d3dVertexBuffer);
	SafeRelease(m_d3dIndexBuffer);
	SafeRelease(m_d3dVertexShader);
	SafeRelease(m_d3dPixelShader);
	SafeRelease(m_d3dHullShader);
	SafeRelease(m_d3dDomainShader);
	SafeRelease(m_d3dVertexShaderConstantBuffers);
	SafeRelease(m_d3dPixelShaderConstantBuffers);
	SafeRelease(m_d3dHullShaderConstantBuffers);
	SafeRelease(m_d3dDomainShaderConstantBuffers);

	SafeRelease(m_d3dSamplerState);

	m_Vertices.clear();
	m_Indices.clear();
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
bool Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::Initialize()
{
	m_Initialized = false;

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
		vertexShaderBlob->Release();
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
			vertexShaderBlob->Release();
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
			vertexShaderBlob->Release();
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
		vertexShaderBlob->Release();
		return false;
	}

	vertexShaderBlob->Release();

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
	GetVertexShaderConstantBufferDesc(constantBufferDesc);

	if (constantBufferDesc.ByteWidth > 1)
	{
		if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			&m_d3dVertexShaderConstantBuffers)))
		{
			cout << "D3D11: Failed to create per-object vertex shader constant buffer\n";
			return false;
		}
	}

	D3D11_BUFFER_DESC pixelShaderBufferDesc = {};
	GetPixelShaderConstantBufferDesc(pixelShaderBufferDesc);

	if (pixelShaderBufferDesc.ByteWidth > 1)
	{
		if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateBuffer(
			&pixelShaderBufferDesc,
			nullptr,
			&m_d3dPixelShaderConstantBuffers)))
		{
			cout << "D3D11: Failed to create per-object pixel shader buffer\n";
			return false;
		}
	}

	D3D11_BUFFER_DESC hullShaderBufferDesc = {};
	GetHullShaderConstantBufferDesc(hullShaderBufferDesc);

	if (hullShaderBufferDesc.ByteWidth > 1)
	{
		if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateBuffer(
			&hullShaderBufferDesc,
			nullptr,
			&m_d3dHullShaderConstantBuffers)))
		{
			cout << "D3D11: Failed to create per-object hull shader buffer\n";
			return false;
		}
	}

	D3D11_BUFFER_DESC domainShaderBufferDesc = {};
	GetDomainShaderConstantBufferDesc(domainShaderBufferDesc);

	if (domainShaderBufferDesc.ByteWidth > 1)
	{
		if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateBuffer(
			&domainShaderBufferDesc,
			nullptr,
			&m_d3dDomainShaderConstantBuffers)))
		{
			cout << "D3D11: Failed to create per-object domain shader buffer\n";
			return false;
		}
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

	m_Initialized = true;

	return true;
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
void Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::Start()
{

}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
void Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::Update()
{
	if (m_d3dVertexShaderConstantBuffers != nullptr) D3D11Application::GetInstance().GetDeviceContext()->UpdateSubresource(m_d3dVertexShaderConstantBuffers, 0, nullptr, &m_VertexShaderConstantBufferData, 0, 0);
	if (m_d3dHullShaderConstantBuffers != nullptr) D3D11Application::GetInstance().GetDeviceContext()->UpdateSubresource(m_d3dHullShaderConstantBuffers, 0, nullptr, &m_HullShaderConstantBufferData, 0, 0);
	if (m_d3dDomainShaderConstantBuffers != nullptr) D3D11Application::GetInstance().GetDeviceContext()->UpdateSubresource(m_d3dDomainShaderConstantBuffers, 0, nullptr, &m_DomainShaderConstantBufferData, 0, 0);
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
void Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::GetVertexShaderConstantBufferDesc(D3D11_BUFFER_DESC& constantBufferDesc)
{
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(VertexShaderConstantBufferData);
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
void Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::GetPixelShaderConstantBufferDesc(D3D11_BUFFER_DESC& constantBufferDesc)
{
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(PixelShaderConstantBufferData);
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
void Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::GetHullShaderConstantBufferDesc(D3D11_BUFFER_DESC& constantBufferDesc)
{
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(HullShaderConstantBufferData);
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
void Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::GetDomainShaderConstantBufferDesc(D3D11_BUFFER_DESC& constantBufferDesc)
{
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(DomainShaderConstantBufferData);
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
ID3D11ShaderResourceView* const* Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::GetVertexShaderSRVs()
{
	return nullptr;
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
ID3D11ShaderResourceView* const* Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::GetPixelShaderSRVs()
{
	return nullptr;
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
ID3D11ShaderResourceView* const* Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::GetHullShaderSRVs()
{
	return nullptr;
}

template<typename VertexData, typename VertexShaderConstantBufferData, typename PixelShaderConstantBufferData, typename HullShaderConstantBufferData, typename DomainShaderConstantBufferData>
ID3D11ShaderResourceView* const* Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>::GetDomainShaderSRVs()
{
	return nullptr;
}