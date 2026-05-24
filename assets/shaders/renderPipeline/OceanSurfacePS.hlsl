
#define PI 3.14159265359
#define SQRT_2 1.414213562

struct PSInput
{
    float3 WorldPosition : TEXCOORD0;
    float2 UVs[4] : TEXCOORD1;
    float3 ViewVector : TEXCOORD5;
    float3 EyePos : TEXCOORD6;
    float3 Color : COLOR;
    float4 Position : SV_POSITION;
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
    float m_DecayFactor;
    float3 m_SpecularColor;
    float m_FoamAddition;
    float3 m_FogColor;
    float m_FogDistance;
    
    float3 m_UpwellingColor;
    float m_Snell;
    float3 m_AirColor;
    float m_kDiffuse;
}

Texture2D SlopeTextureCascade[4] : register(t0);
Texture2D SecondOrderMomentsCascade[4] : register(t4);
TextureCube SkyboxTexture : register(t8);
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

float BeckmannDistribution(float2 halfVectorSlopes, float2 averageSlope, float2x2 covarianceMatrix)
{
    float2 difference = halfVectorSlopes - averageSlope;
    float determinant = max(covarianceMatrix._11 * covarianceMatrix._22 - covarianceMatrix._12 * covarianceMatrix._21, 0.00001f);
    
    float matrixMultiplication = (difference.x * difference.x * covarianceMatrix._22 + difference.y * difference.y * covarianceMatrix._11 - 2.0f * difference.x * difference.y * covarianceMatrix._12) / determinant;

    float exponent = exp(-0.5f * matrixMultiplication);

    return 1.0f / (2.0f * PI * sqrt(determinant)) * exponent;
}

float Smith(float3 dir, float2 averageSlopes, float2x2 covarianceMatrix)
{        
    if (dir.y <= 0.000001f)
        return 9999.0f;
    
    float muPhi = dir.x * averageSlopes.x + dir.z * averageSlopes.y;
    
    float sigmaPhiSquare = dir.x * dir.x * covarianceMatrix._11 + dir.z * dir.z * covarianceMatrix._22 + 2.0f * dir.x * dir.z * covarianceMatrix._12;
    
    sigmaPhiSquare = max(sigmaPhiSquare, 0.000001f);
    
    float v = (dir.y - muPhi) / (sqrt(sigmaPhiSquare) * SQRT_2);
    
    return v < 1.6 ? (1.0f - 1.259f * v + 0.396f * v * v) / (3.535f * v + 2.181f * v * v) : 0.0f;
}

float MaskingShadowing(float3 viewDir, float3 lightDir, float2 averageSlopes, float2x2 covarianceMatrix)
{    
    float smithView = Smith(viewDir, averageSlopes, covarianceMatrix);
    
    float smithLight = Smith(lightDir, averageSlopes, covarianceMatrix);

    return 1.0f / (1.0f + smithView + smithLight);
}

float NormalDistribution(float p22, float3 halfVector, float3 macronormal)
{
    return p22 / max(pow(dot(halfVector, macronormal), 4), 0.00001f);
}

float Fresnel(float3 viewDir, float3 halfVector, float2x2 covarianceMatrix)
{
    float R = pow(m_Snell - 1.0f, 2) / pow(m_Snell + 1.0f, 2);
    
    float length = sqrt(viewDir.x * viewDir.x + viewDir.z * viewDir.z);
    
    length = max(length, 0.000001f);
    
    float cosPhi = viewDir.x / length;
    float sinPhi = viewDir.z / length;
    
    float projectedRoughness = sqrt(covarianceMatrix._11 * cosPhi * cosPhi + covarianceMatrix._22 * sinPhi * sinPhi + 2.0f * cosPhi * sinPhi * covarianceMatrix._12) * SQRT_2;

    float cosTheta = dot(viewDir, halfVector);
    
    return R + (1.0f - R) * pow(1.0f - cosTheta, 5.0f * exp(-2.69f * projectedRoughness)) / (1.0f + 22.7f * pow(projectedRoughness, 1.5f));
}

PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
    
    float3 lightDir = normalize(m_LightDirection);
    
    float2 totalSlope = float2(0.0f, 0.0f);
    float3 totalSecondOrderMoments = float3(0.0f, 0.0f, 0.0f);
    float totalFoam = 0.0f;

    for (int i = 0; i < 4; i++)
    {
        float4 slopeSample = SlopeTextureCascade[i].Sample(LinearSampler, input.UVs[i]);
        float4 secondOrderMomentsSample = SecondOrderMomentsCascade[i].Sample(LinearSampler, input.UVs[i]);
    
        totalSecondOrderMoments.x = totalSecondOrderMoments.x + secondOrderMomentsSample.x + 2.0f * totalSlope.x * slopeSample.x;
        totalSecondOrderMoments.y = totalSecondOrderMoments.y + secondOrderMomentsSample.y + 2.0f * totalSlope.y * slopeSample.y;
        totalSecondOrderMoments.z = totalSecondOrderMoments.z + secondOrderMomentsSample.z + totalSlope.x * slopeSample.y + totalSlope.y * slopeSample.x;
    
        totalSlope += slopeSample.xy;
        
        totalFoam += slopeSample.a;
    }
    
    float xVariance = totalSecondOrderMoments.x - totalSlope.x * totalSlope.x;
    float yVariance = totalSecondOrderMoments.y - totalSlope.y * totalSlope.y;
    float xyCovariance = totalSecondOrderMoments.z - totalSlope.x * totalSlope.y;
    
    float2x2 covarianceMatrix = float2x2(xVariance, xyCovariance, xyCovariance, yVariance);
    
    // Reconstruct the final normal from the combined slopes
    float3 normal = normalize(float3(-totalSlope.x, 1.0f, -totalSlope.y));

    // Calculate final foam
    float foam = saturate(totalFoam);
    
    //float3 normal = normalize(slopeSample.xyz);
    float3 viewDir = normalize(input.ViewVector);
    float3 halfVector = normalize(-lightDir + viewDir);
    float3 macronormal = float3(0.0f, 1.0f, 0.0f);
    
    float2 halfVectorSlopes = float2(-halfVector.x / halfVector.y, -halfVector.z / halfVector.y);
    
    float p22 = BeckmannDistribution(halfVectorSlopes, totalSlope, covarianceMatrix);
    
    float D = NormalDistribution(p22, halfVector, macronormal);
    
    float maskingShadowingValue = MaskingShadowing(viewDir, -lightDir, totalSlope, covarianceMatrix);
    
    float fresnel = Fresnel(viewDir, halfVector, covarianceMatrix);
    
    float specularColor = m_LightColor * fresnel * D * maskingShadowingValue / (4.0f * max(dot(macronormal, viewDir), 0.000001f));
    
    //float3 finalColor = specularColor;
    
    //float fresnel = pow(1.0 - max(0, dot(viewDir, normal)), 5);
    
    float3 ambientLight = m_AmbientLightIntensity * m_LightColor * 0.0f;
    float3 diffuseLight = m_LightColor * max(0, dot(normal, -lightDir));
    float3 specularLight = m_SpecularColor * pow(max(0, dot(halfVector, normal)), 32) * fresnel;
    
    float3 reflectedViewDir = normalize(reflect(viewDir, normal));
    float3 environmentalReflection = SkyboxTexture.Sample(LinearSampler, reflectedViewDir).xyz * m_AmbientLightIntensity * fresnel;
    
    float3 finalColor = ambientLight + diffuseLight * input.Color + specularLight * fresnel + environmentalReflection;
    
    //if (input.Position.x > 1920 * 0.45)
    //    finalColor = TessendorfLighting(normal, lightDir, viewDir, m_LightColor, input.WorldPosition, input.EyePos, m_Snell, m_kDiffuse);
    
    //float foam = saturate(slopeSample.a);
    
    finalColor = lerp(finalColor, m_FoamColor, foam); //clamp((m_FoamBias - slopeSample.a) / m_FoamBias, 0, 1));
    
    //finalColor += m_FogColor * saturate(length(input.WorldPosition - input.EyePos) / m_FogDistance);
    
    output.Color = float4(finalColor, 1.0);
   
    return output;
}