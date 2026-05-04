#include "Texture2D.h"
#include "D3D11Application.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture2D::Texture2D(int textureCount, string* path, bool useSRV, bool useUAV)
{
	m_TextureCount = textureCount;

	m_TexturePath = path;
	m_UseSRV = useSRV;
	m_UseUAV = useUAV;
    m_LoadFromFile = true;

    m_TextureWidth = 0;
    m_TextureHeight = 0;

    InitializeTextureArrays();
}

Texture2D::Texture2D(int textureCount, UINT width, UINT height, bool useSRV, bool useUAV)
{
    m_TextureCount = textureCount;

    m_TexturePath = { nullptr };
    m_UseSRV = useSRV;
    m_UseUAV = useUAV;
    m_LoadFromFile = false;

    m_TextureWidth = width;
    m_TextureHeight = height;

	InitializeTextureArrays();
}

Texture2D::~Texture2D()
{
    ReleaseTextureResources();
}

bool Texture2D::Initialize()
{
    ID3D11Device* device = D3D11Application::GetInstance().GetDevice();

    if (!m_LoadFromFile)
    {
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = m_TextureWidth;
        texDesc.Height = m_TextureHeight;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = 0;

        if (m_UseSRV) texDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        if (m_UseUAV) texDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = texDesc.Format;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

        for (int i = 0; i < m_TextureCount; i++)
        {
            if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_Texture[i])))
            {
                cout << "Failed to create texture! \n";
                return false;
            }

            if (m_UseSRV)
            {
                if (FAILED(device->CreateShaderResourceView(m_Texture[i], &srvDesc, &m_TextureSRV[i])))
                {
                    cout << "Failed to create texture SRV! \n";
                    return false;
                }
            }

            if (m_UseUAV)
            {
                if (FAILED(device->CreateUnorderedAccessView(m_Texture[i], &uavDesc, &m_TextureUAV[i])))
                {
                    cout << "Failed to create texture UAV! \n";
                    return false;
                }
            }
        }
    }
    else
    {
        int imgWidth, imgHeight, imgChannels;


        for (int i = 0; i < m_TextureCount; i++)
        {
#if _DEBUG
			string path = m_TexturePath[i];
#else
			string path = "../../" + m_TexturePath[i];
#endif

            // Force the image to load with 4 channels (RGBA) so it aligns perfectly with DirectX formats
            unsigned char* rawData = stbi_load(path.c_str(), &imgWidth, &imgHeight, &imgChannels, 4);

            if (!rawData)
            {
                cout << "Failed to read texture data from file! \n";
                return false;
            }

            m_TextureWidth = static_cast<UINT>(imgWidth);
            m_TextureHeight = static_cast<UINT>(imgHeight);

            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = m_TextureWidth;
            texDesc.Height = m_TextureHeight;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            texDesc.SampleDesc.Count = 1;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = 0;

			if (m_UseSRV) texDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            if (m_UseUAV) texDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = rawData;
            initData.SysMemPitch = m_TextureWidth * 4; // 4 bytes per pixel (R, G, B, A)

            if (FAILED(device->CreateTexture2D(&texDesc, &initData, &m_Texture[i])))
            {
                cout << "Failed to create texture from file! \n";
                stbi_image_free(rawData);
                return false;
            }

            stbi_image_free(rawData);

            if (m_UseSRV)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = texDesc.Format;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = 1;

                if (FAILED(device->CreateShaderResourceView(m_Texture[i], &srvDesc, &m_TextureSRV[i])))
                {
                    cout << "Failed to create texture SRV from file! \n";
                    return false;
                }
            }

            if (m_UseUAV)
            {
                D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.Format = texDesc.Format;
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

                if (FAILED(device->CreateUnorderedAccessView(m_Texture[i], &uavDesc, &m_TextureUAV[i])))
                {
                    cout << "Failed to create texture UAV from file! \n";
                    return false;
                }
            }
        }
    }

    return true;
}

void Texture2D::ReleaseTextureResources()
{
    for (int i = 0; i < m_TextureCount; i++)
    {
        if (m_Texture && m_Texture[i]) m_Texture[i]->Release();
        if (m_TextureSRV && m_TextureSRV[i]) m_TextureSRV[i]->Release();
        if (m_TextureUAV && m_TextureUAV[i]) m_TextureUAV[i]->Release();
    }

    if (m_Texture) { delete[] m_Texture; m_Texture = nullptr; };
    if (m_TextureSRV) { delete[] m_TextureSRV; m_TextureSRV = nullptr; }
    if (m_TextureUAV) { delete[] m_TextureUAV; m_TextureUAV = nullptr; }
}

bool Texture2D::ResizeTexture(UINT width, UINT height)
{
    if (!m_LoadFromFile)
    {
        m_TextureWidth = width;
        m_TextureHeight = height;

        ReleaseTextureResources();

		InitializeTextureArrays();

        return Initialize();
    }

    return true;
}

void Texture2D::InitializeTextureArrays()
{
    m_Texture = new ID3D11Texture2D * [m_TextureCount] {};
    m_TextureSRV = nullptr;
    m_TextureUAV = nullptr;

    if (m_UseSRV) m_TextureSRV = new ID3D11ShaderResourceView * [m_TextureCount] {};
    if (m_UseUAV) m_TextureUAV = new ID3D11UnorderedAccessView * [m_TextureCount] {};
}