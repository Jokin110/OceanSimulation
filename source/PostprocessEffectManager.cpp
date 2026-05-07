#include "PostprocessEffectManager.h"
#include "DefaultPostprocessEffect.h"

PostprocessEffectManager* PostprocessEffectManager::m_Instance = nullptr;

PostprocessEffectManager::PostprocessEffectManager()
{
	m_PostprocessEffects.clear();

	m_VertexShaderFile = L"assets/shaders/postprocessEffects/PostprocessEffectVS.hlsl";

	m_d3dInputLayout = nullptr;
	m_d3dVertexBuffer = nullptr;
	m_d3dIndexBuffer = nullptr;
	m_d3dVertexShader = nullptr;
}

PostprocessEffectManager::~PostprocessEffectManager()
{
	ReleaseResources();

	for (int i = 0; i < m_PostprocessEffects.size(); i++)
	{
		delete m_PostprocessEffects[i];
		m_PostprocessEffects[i] = nullptr;
	}

	m_PostprocessEffects.clear();

	if (m_Instance)
	{
		m_Instance = nullptr;
	}
}

void PostprocessEffectManager::ReleaseResources()
{
	SafeRelease(m_d3dInputLayout);
	SafeRelease(m_d3dVertexBuffer);
	SafeRelease(m_d3dIndexBuffer);
	SafeRelease(m_d3dVertexShader);

	DestroyRenderTargetResources();

	m_Vertices.clear();
	m_Indices.clear();
}

bool PostprocessEffectManager::Initialize()
{
	if (!m_Instance)
	{
		m_Instance = new PostprocessEffectManager;

		m_Instance->GenerateMesh();

#if _DEBUG
		wstring pathPrefix = L"";
#else
		wstring pathPrefix = L"../../";
#endif

		ID3DBlob* vertexShaderBlob = nullptr;
		m_Instance->m_d3dVertexShader = D3D11Application::GetInstance().CreateVertexShader(pathPrefix + m_Instance->m_VertexShaderFile, vertexShaderBlob);

		if (m_Instance->m_d3dVertexShader == nullptr)
		{
			return false;
		}

		D3D11_INPUT_ELEMENT_DESC vertexInputLayout[] =
		{
			{
				"TEXCOORD",
				0,
				DXGI_FORMAT_R32G32_FLOAT,
				0,
				offsetof(VertexDataPostprocessEffect, m_UV),
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			}
		};

		if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateInputLayout(
			vertexInputLayout,
			_countof(vertexInputLayout),
			vertexShaderBlob->GetBufferPointer(),
			vertexShaderBlob->GetBufferSize(),
			&m_Instance->m_d3dInputLayout)))
		{
			cout << "D3D11: Failed to create default vertex input layout\n";
			vertexShaderBlob->Release();
			return false;
		}

		vertexShaderBlob->Release();

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.ByteWidth = m_Instance->GetVerticesByteWidth();
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA resourceData = {};
		resourceData.pSysMem = m_Instance->m_Vertices.data();

		if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateBuffer(
			&vertexBufferDesc,
			&resourceData,
			&m_Instance->m_d3dVertexBuffer)))
		{
			cout << "D3D11: Failed to create triangle vertex buffer\n";
			return false;
		}

		D3D11_BUFFER_DESC indexBufferDesc = {};

		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.ByteWidth = m_Instance->GetIndicesByteWidth();
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		resourceData.pSysMem = m_Instance->m_Indices.data();

		if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateBuffer(
			&indexBufferDesc,
			&resourceData,
			&m_Instance->m_d3dIndexBuffer)))
		{
			cout << "D3D11: Failed to create triangle index buffer\n";
			return false;
		}

		m_Instance->CreateRenderTargetResources();

		return true;
	}

	return false;
}

bool PostprocessEffectManager::CreateRenderTargetResources()
{
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = D3D11Application::GetInstance().GetWindowWidth();
	texDesc.Height = D3D11Application::GetInstance().GetWindowHeight();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	for (int i = 0; i < 2; i++)
	{
		if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateTexture2D(&texDesc, nullptr, &m_Instance->m_d3dPostprocessPingPongBuffer[i])))
		{
			cout << "D3D11: Failed to create render target texture." << endl;
			return false;
		}

		if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateRenderTargetView(m_Instance->m_d3dPostprocessPingPongBuffer[i], &rtvDesc, &m_Instance->m_d3dPostprocessPingPongRTV[i])))
		{
			cout << "D3D11: Failed to create render target view for the scene color texture." << endl;
			return false;
		}

		if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateShaderResourceView(m_Instance->m_d3dPostprocessPingPongBuffer[i], &srvDesc, &m_Instance->m_d3dPostprocessPingPongSRV[i])))
		{
			cout << "D3D11: Failed to create shader resource view for the scene color texture." << endl;
			return false;
		}
	}

	return true;
}

void PostprocessEffectManager::DestroyRenderTargetResources()
{
	for (int i = 0; i < 2; i++)
	{
		SafeRelease(m_d3dPostprocessPingPongBuffer[i]);
		SafeRelease(m_d3dPostprocessPingPongRTV[i]);
		SafeRelease(m_d3dPostprocessPingPongSRV[i]);
	}
}

bool PostprocessEffectManager::OnResize()
{
	DestroyRenderTargetResources();
	return CreateRenderTargetResources();
}

bool PostprocessEffectManager::InitializeEffects()
{
	if (m_PostprocessEffects.size() == 0)
	{
		new DefaultPostprocessEffect("Default Postprocess Effect", L"assets/shaders/postprocessEffects/DefaultPostprocessEffect.hlsl");
	}

	for (int i = 0; i < m_PostprocessEffects.size(); i++)
	{
		if (!m_PostprocessEffects[i]->Initialize())
			return false;
	}

	return true;
}

void PostprocessEffectManager::AddEffectToList(IPostprocessEffect* effect)
{
	m_PostprocessEffects.push_back(effect);
}

void PostprocessEffectManager::Start()
{
	for (int i = 0; i < m_PostprocessEffects.size(); i++)
	{
		m_PostprocessEffects[i]->Start();
	}
}

void PostprocessEffectManager::Update()
{
	for (int i = 0; i < m_PostprocessEffects.size(); i++)
	{
		m_PostprocessEffects[i]->Update();
	}
}

void PostprocessEffectManager::Render()
{
	ID3D11DeviceContext* deviceContext = D3D11Application::GetInstance().GetDeviceContext();

	deviceContext->ClearRenderTargetView(m_d3dPostprocessPingPongRTV[0], Colors::SkyBlue);
	deviceContext->ClearRenderTargetView(m_d3dPostprocessPingPongRTV[1], Colors::SkyBlue);

	int pingPongIndexSrc = 0;
	int pingPongIndexDst = 1;

	for (int i = 0; i < m_PostprocessEffects.size(); i++)
	{
		pingPongIndexSrc = i % 2;
		pingPongIndexDst = 1 - pingPongIndexSrc;

		if (i == m_PostprocessEffects.size() - 1)
		{
			ID3D11RenderTargetView* backBufferRTV = D3D11Application::GetInstance().GetRenderTargetView();
			deviceContext->OMSetRenderTargets(1, &backBufferRTV, nullptr);
		}
		else
		{
			deviceContext->OMSetRenderTargets(1, &m_d3dPostprocessPingPongRTV[pingPongIndexDst], nullptr);
		}

		m_PostprocessEffects[i]->Render();

		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		const UINT vertexStride = GetVertexStride();
		constexpr UINT vertexOffset = 0;

		deviceContext->IASetInputLayout(m_d3dInputLayout);

		deviceContext->IASetVertexBuffers(0, 1, &m_d3dVertexBuffer, &vertexStride, &vertexOffset);

		deviceContext->IASetIndexBuffer(m_d3dIndexBuffer, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

		// Vertex shader
		deviceContext->VSSetShader(m_d3dVertexShader, nullptr, 0);

		// Pixel shader
		deviceContext->PSSetShader(m_PostprocessEffects[i]->GetPixelShader(), nullptr, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &m_PostprocessEffects[i]->GetPixelShaderConstantBuffers());

		if (i == 0)
		{
			ID3D11ShaderResourceView* sceneColorSRV = D3D11Application::GetInstance().GetSceneColorSRV();
			deviceContext->PSSetShaderResources(0, 1, &sceneColorSRV);
		}
		else
		{
			deviceContext->PSSetShaderResources(0, 1, &m_d3dPostprocessPingPongSRV[pingPongIndexSrc]);
		}

		if (m_PostprocessEffects[i]->GetPixelShaderSRVCount() > 0)
		{
			deviceContext->PSSetShaderResources(1, m_PostprocessEffects[i]->GetPixelShaderSRVCount(), m_PostprocessEffects[i]->GetPixelShaderSRVs());
		}

		deviceContext->PSSetSamplers(0, 1, &m_PostprocessEffects[i]->GetSamplerState());

		// Draw the object
		deviceContext->DrawIndexed(GetIndexCount(), 0, 0);

		// Unbind shader resources after drawing the object to prevent them from being used by other objects
		ID3D11ShaderResourceView* nullSRVs[16] = { nullptr };

		UINT psCount = m_PostprocessEffects[i]->GetPixelShaderSRVCount() + 1;
		if (psCount > 0) deviceContext->PSSetShaderResources(0, psCount, nullSRVs);
	}
}

void PostprocessEffectManager::GenerateMesh()
{
	m_Vertices.clear();
	m_Indices.clear();

	// Define the UV coordinates for a face
	XMFLOAT2 uvTopLeft = XMFLOAT2(0.0f, 0.0f);
	XMFLOAT2 uvTopRight = XMFLOAT2(1.0f, 0.0f);
	XMFLOAT2 uvBotLeft = XMFLOAT2(0.0f, 1.0f);
	XMFLOAT2 uvBotRight = XMFLOAT2(1.0f, 1.0f);

	m_Vertices.push_back({ uvTopLeft });
	m_Vertices.push_back({ uvTopRight });
	m_Vertices.push_back({ uvBotLeft });
	m_Vertices.push_back({ uvBotRight });

	m_Indices.push_back(0);
	m_Indices.push_back(1);
	m_Indices.push_back(2);
	m_Indices.push_back(1);
	m_Indices.push_back(3);
	m_Indices.push_back(2);
}