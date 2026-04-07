
struct PSInput
{
    float3 WorldPosition : TEXCOORD0;
    float3 NormalWS : NORMAL;
    float3 ViewVector : TEXCOORD2;
    float3 EyePos : TEXCOORD3;
    float Jacobian : JACOBIAN;
    float3 Color : COLOR;
    float4 Position : SV_Position;
};

struct PSOutput
{
    float4 Color : SV_Target0;
};

cbuffer RenderingValuesBuffer : register(b0)
{
    float3 m_FoamColor;
    float m_FoamBias;
    float3 m_LightColor;
    float m_AmbientLightIntensity;
    float3 m_LightDirection;
    float m_Padding0;
    float3 m_SpecularColor;
    float m_Padding1;
    float3 m_FogColor;
    float m_FogDistance;
    
    float3 m_UpwellingColor;
    float m_Snell;
    float3 m_AirColor;
    float m_kDiffuse;
}

Texture2D InitialSpectrum : register(t0);
SamplerState LinearSampler : register(s0);

float3 TessendorfLighting(float3 normal, float3 lightDir, float3 viewDir, float3 skyColor, float3 P, float3 E, float nSnell = 1.33, float kDiffuse = 0.0)
{    
    float reflectivity = 0.0;
    float costhetai = abs(dot(lightDir, normal));
    float thetai = acos(costhetai);
    float sinthetat = sin(thetai) / nSnell;
    float thetat = asin(sinthetat);
    
    if (thetai == 0.0)
    {
        reflectivity = (nSnell - 1) / (nSnell + 1);
        reflectivity = reflectivity * reflectivity;
    }
    else
    {
        float fs = sin(thetat - thetai) / sin(thetat + thetai);
        float ts = tan(thetat - thetai) / tan(thetat + thetai);
        reflectivity = 0.5 * (fs * fs + ts * ts);
    }
    
    float3 dPE = P - E;
    float dist = length(dPE) * kDiffuse;
    dist = exp(-dist);
    
    float fresnel = pow(1.0 - max(0, dot(viewDir, normal)), 5);
    
    float3 color = dist * (reflectivity * skyColor + (1 - reflectivity) * m_UpwellingColor) + (1 - dist) * m_AirColor;
        
    return color;
}

PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
    
    float3 lightDir = normalize(m_LightDirection);
    
    float3 normal = normalize(input.NormalWS);
    float3 viewDir = normalize(input.ViewVector);
    float3 halfVector = normalize(-lightDir + viewDir);
    
    float3 ambientLight = m_AmbientLightIntensity * m_LightColor;
    float3 diffuseLight = m_LightColor * max(0, dot(normal, -lightDir));
    float3 specularLight = m_SpecularColor * pow(max(0, dot(halfVector, normal)), 32);
    float fresnel = pow(1.0 - max(0, dot(viewDir, normal)), 5);
    
    float3 finalColor = ambientLight + diffuseLight * input.Color + specularLight * fresnel;
    
    if (input.Position.x > 1920 * 0.45)
        finalColor = TessendorfLighting(normal, lightDir, viewDir, m_LightColor, input.WorldPosition, input.EyePos, m_Snell, m_kDiffuse);
    
    finalColor = lerp(finalColor, m_FoamColor, clamp((m_FoamBias - input.Jacobian) / m_FoamBias, 0, 1));
    
    finalColor += m_FogColor * saturate(length(input.WorldPosition - input.EyePos) / m_FogDistance);
    
    output.Color = float4(finalColor, 1.0);
    
    output.Color = InitialSpectrum.SampleLevel(LinearSampler, input.Position.xy / float2(1920.0f, 1080.0f) / 0.9f, 0);
    
    //output.Color = float4(1.0, 1.0, 1.0, 1.0);
    
    return output;
}