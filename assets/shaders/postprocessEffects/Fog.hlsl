
struct PSInput
{
    float2 m_UV : TEXCOORD0;
	float4 m_Position : SV_POSITION;
};

cbuffer FogSettingsBuffer : register (b0)
{
    row_major matrix m_InverseViewProjectionMatrix;
    float3 m_CameraPosition;
    float m_FogDensity;
	float3 m_FogColor;
	float m_HeightFalloff;
    float3 m_LightDirection;
    float m_FogFactorExponent;
    float3 m_LightColor;
    float m_LightScatteringIntensity;
};

Texture2D SceneColorTexture : register(t0);
Texture2D DepthTexture : register(t1);

SamplerState PointSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float depth = DepthTexture.Sample(PointSampler, input.m_UV).r;
    float3 sceneColor = SceneColorTexture.Sample(PointSampler, input.m_UV).rgb;
    
    float2 normalizedPos = float2(input.m_UV.x * 2.0f - 1.0f, 1.0f - input.m_UV.y * 2.0f);
    float3 ndc = float3(normalizedPos, depth);
    float4 homogeneousWorldSpace = mul(float4(ndc, 1.0f), m_InverseViewProjectionMatrix);
    float3 worldSpacePos = homogeneousWorldSpace.xyz / homogeneousWorldSpace.w;
    
    float3 viewDir = normalize(m_CameraPosition - worldSpacePos);

    float distance = length(worldSpacePos - m_CameraPosition);
	
    float heightFactor = exp(-m_HeightFalloff * worldSpacePos.y);
    float fogFactor = exp(-pow(distance * m_FogDensity * heightFactor, m_FogFactorExponent));

    fogFactor = saturate(1.0f - fogFactor);
    
    float lightScattering = saturate(dot(viewDir, m_LightDirection));
    lightScattering = pow(lightScattering, m_LightScatteringIntensity);
    lightScattering = 0.0f;
    
    float3 fogColor = lerp(m_FogColor, m_LightColor, lightScattering);

    sceneColor = lerp(sceneColor, fogColor, fogFactor);

	return float4(sceneColor, 1.0f);
}