#pragma once

#include <d3d11.h>
#include <string>

using namespace std;

class Texture2D
{
public:
	Texture2D(int textureCount, string* paths, bool useSRV = true, bool useUAV = false);
	Texture2D(int textureCount, UINT width, UINT height, bool useSRV = true, bool useUAV = true);
	~Texture2D();

	bool Initialize();
	void ReleaseTextureResources();
	bool ResizeTexture(UINT width, UINT height);

	ID3D11ShaderResourceView* const* GetTextureSRVs() { return m_TextureSRV; }
	ID3D11UnorderedAccessView* const* GetTextureUAVs() { return m_TextureUAV; }

private:
	int m_TextureCount = 1;

	string* m_TexturePath = { nullptr };
	bool m_LoadFromFile = false;
	bool m_UseSRV = true;
	bool m_UseUAV = true;

	UINT m_TextureWidth = 256;
	UINT m_TextureHeight = 256;

	ID3D11Texture2D** m_Texture = { nullptr };
	ID3D11ShaderResourceView** m_TextureSRV = { nullptr };
	ID3D11UnorderedAccessView** m_TextureUAV = { nullptr };

	void InitializeTextureArrays();
};
