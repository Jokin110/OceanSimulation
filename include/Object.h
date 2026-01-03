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

	virtual ID3D11Buffer*& GetConstantBuffers() = 0;

	virtual UINT GetIndexCount() = 0;

	virtual UINT GetVertexStride() = 0;
};

template<typename VertexData, typename ConstantBufferData>
class Object : IObject
{
public:
	Object();
	Object(string name);
	Object(string name, wstring vertexShaderFile, wstring pixelShaderFile);
	~Object();

	virtual bool Initialize();
	virtual void Start();
	virtual void Update();

	ID3D11InputLayout*& GetInputLayout() { return m_d3dInputLayout; }
	ID3D11Buffer*& GetVertexBuffer() { return m_d3dVertexBuffer; }
	ID3D11Buffer*& GetIndexBuffer() { return m_d3dIndexBuffer; }

	ID3D11VertexShader*& GetVertexShader() { return m_d3dVertexShader; }
	ID3D11PixelShader*& GetPixelShader() { return m_d3dPixelShader; }

	ID3D11Buffer*& GetConstantBuffers() { return m_d3dConstantBuffers; }

	UINT GetIndexCount() { return static_cast<UINT>(m_Indices.size()); }

	UINT GetVertexStride() { return sizeof(VertexData); }

protected:
	string m_Name;
	Vector3 m_Position;
	Vector3 m_Rotation;
	Vector3 m_Scale;

	wstring m_VertexShaderFile;
	wstring m_PixelShaderFile;

	virtual UINT GetVertexInputLayout(D3D11_INPUT_ELEMENT_DESC*& inputLayout) = 0;
	void GetConstantBufferDesc(D3D11_BUFFER_DESC& constantBufferDesc);

	vector<VertexData> m_Vertices;
	vector<int> m_Indices;

	ConstantBufferData m_ConstantBufferData;

	virtual void GenerateMesh() = 0;

private:
	ID3D11InputLayout* m_d3dInputLayout = nullptr;
	ID3D11Buffer* m_d3dVertexBuffer = nullptr;
	ID3D11Buffer* m_d3dIndexBuffer = nullptr;

	ID3D11VertexShader* m_d3dVertexShader = nullptr;
	ID3D11PixelShader* m_d3dPixelShader = nullptr;

	ID3D11Buffer* m_d3dConstantBuffers = nullptr;

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

template<typename VertexData, typename ConstantBufferData>
Object<VertexData, ConstantBufferData>::Object()
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
	m_d3dConstantBuffers = nullptr;

	m_ConstantBufferData = {};

	ObjectManager::GetInstance().AddObjectToList(this);
}

template<typename VertexData, typename ConstantBufferData>
Object<VertexData, ConstantBufferData>::Object(string name) : Object()
{
	m_Name = name;
}

template<typename VertexData, typename ConstantBufferData>
Object<VertexData, ConstantBufferData>::Object(string name, wstring vertexShaderFile, wstring pixelShaderFile) : Object(name)
{
	m_VertexShaderFile = vertexShaderFile;
	m_PixelShaderFile = pixelShaderFile;
}

template<typename VertexData, typename ConstantBufferData>
Object<VertexData, ConstantBufferData>::~Object()
{
	SafeRelease(m_d3dInputLayout);
	SafeRelease(m_d3dVertexBuffer);
	SafeRelease(m_d3dIndexBuffer);
	SafeRelease(m_d3dVertexShader);
	SafeRelease(m_d3dPixelShader);
	SafeRelease(m_d3dConstantBuffers);

	m_Vertices.clear();
	m_Indices.clear();
}

template<typename VertexData, typename ConstantBufferData>
bool Object<VertexData, ConstantBufferData>::Initialize()
{
	GenerateMesh();

	ID3DBlob* vertexShaderBlob = nullptr;
#if _DEBUG
	m_d3dVertexShader = D3D11Application::GetInstance().CreateVertexShader(m_VertexShaderFile, vertexShaderBlob);
#else
	m_d3dVertexShader = CreateVertexShader(L"../../" + m_VertexShaderFile, vertexShaderBlob);
#endif

	if (m_d3dVertexShader == nullptr)
	{
		return false;
	}

#if _DEBUG
	m_d3dPixelShader = D3D11Application::GetInstance().CreatePixelShader(m_PixelShaderFile);
#else
	m_d3dPixelShader = CreatePixelShader(L"../../" + m_PixelShaderFile);
#endif

	if (m_d3dPixelShader == nullptr)
	{
		return false;
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

	return true;
}

template<typename VertexData, typename ConstantBufferData>
void Object<VertexData, ConstantBufferData>::Start()
{

}

template<typename VertexData, typename ConstantBufferData>
void Object<VertexData, ConstantBufferData>::Update()
{
	D3D11Application::GetInstance().GetDeviceContext()->UpdateSubresource(m_d3dConstantBuffers, 0, nullptr, &m_ConstantBufferData, 0, 0);
}

template<typename VertexData, typename ConstantBufferData>
void Object<VertexData, ConstantBufferData>::GetConstantBufferDesc(D3D11_BUFFER_DESC& constantBufferDesc)
{
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(ConstantBufferData);
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
}