#pragma once

#include "PostprocessEffect.h"

// Define the structure of the constant buffers for the pixel shader
struct PixelShaderConstantBufferDataDefaultPostprocessEffect
{
	
};

class DefaultPostprocessEffect : public PostprocessEffect<PixelShaderConstantBufferDataDefaultPostprocessEffect>
{
public:
	DefaultPostprocessEffect(string name, wstring pixelShaderFilePath);
	~DefaultPostprocessEffect();
};

