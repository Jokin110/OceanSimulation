#include "DefaultPostprocessEffect.h"
#include "D3D11Application.h"

DefaultPostprocessEffect::DefaultPostprocessEffect(string name, wstring pixelShaderFilePath) : PostprocessEffect(name, pixelShaderFilePath)
{

}

DefaultPostprocessEffect::~DefaultPostprocessEffect()
{
	ReleaseResources();
}