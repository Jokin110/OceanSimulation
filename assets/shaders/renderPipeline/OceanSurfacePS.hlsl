
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
    
    float m_Snell;
    
    float m_K1;
    float m_K2;
    float m_K3;
    float m_K4;
    
    float3 m_WaterScatterColor;
    float3 m_AirBubblesColor;
    float m_DensityOfAirBubblesSpreadInWater;
}

Texture2D SlopeTextureCascade[4] : register(t0);
Texture2D SecondOrderMomentsCascade[4] : register(t4);
TextureCube SkyboxTexture : register(t8);
SamplerState LinearSampler : register(s0);

float BeckmannDistribution(float2 halfVectorSlopes, float2 averageSlope, float2x2 covarianceMatrix)
{
    float2 difference = halfVectorSlopes - averageSlope;
    float determinant = max(covarianceMatrix._11 * covarianceMatrix._22 - covarianceMatrix._12 * covarianceMatrix._21, 0.00001f);
    
    float matrixMultiplication = (difference.x * difference.x * covarianceMatrix._22 + difference.y * difference.y * covarianceMatrix._11 - 2.0f * difference.x * difference.y * covarianceMatrix._12) / determinant;

    float exponent = exp(-0.5f * matrixMultiplication);

    return 1.0f / (2.0f * PI * sqrt(determinant)) * exponent;
}

float NormalDistribution(float p22, float3 halfVector, float3 macronormal)
{
    return p22 / max(pow(dot(halfVector, macronormal), 4), 0.00001f);
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

float Fresnel(float3 viewDir, float3 halfVector, float2x2 covarianceMatrix)
{
    float R = pow(m_Snell - 1.0f, 2) / pow(m_Snell + 1.0f, 2);
    
    float len = sqrt(viewDir.x * viewDir.x + viewDir.z * viewDir.z);
    
    len = max(len, 0.000001f);
    
    float cosPhi = viewDir.x / len;
    float sinPhi = viewDir.z / len;
    
    float projectedRoughness = sqrt(covarianceMatrix._11 * cosPhi * cosPhi + covarianceMatrix._22 * sinPhi * sinPhi + 2.0f * cosPhi * sinPhi * covarianceMatrix._12) * SQRT_2;

    float cosTheta = clamp(dot(viewDir, halfVector), 0.0f, 1.0f);
    
    return R + (1.0f - R) * pow(1.0f - cosTheta, 5.0f * exp(-2.69f * projectedRoughness)) / (1.0f + 22.7f * pow(projectedRoughness, 1.5f));
}

float3 SpecularColor(float D, float G, float F, float3 normal, float3 macronormal, float3 viewDir)
{
    return dot(normal, macronormal) / max(dot(normal, viewDir), 0.000001f) * m_LightColor * D * G * F / 4.0f;
}

float3 EnvironmentalColor(float2x2 covarianceMatrix, float2 totalSlope, float3 viewDir, float3 normal, float3 macronormal)
{
    float lambda = 1.0f;
    
    float standardDeviationX = sqrt(abs(covarianceMatrix._11));
    float standardDeviationY = sqrt(abs(covarianceMatrix._22));
    
    float correlationCoefficient = covarianceMatrix._12 / max(standardDeviationX * standardDeviationY, 0.000001f);
    
    float3 environmentalColor = 0.0f;
    
    float Wj[3];
    
    float A = pow(2.0f * lambda * max(standardDeviationX, standardDeviationY) / 3.0f, 2);
    
    for (int j = -1; j <= 1; j++)
    {
        float denominator = 0.0f;
        
        for (int k = 1; k >= -1; k--)
        {
            denominator += exp(-0.5f * k * k);
        }
        
        Wj[j + 1] = exp(-0.5f * j * j) / max(denominator, 0.000001);
    }
    
    for (int x = -1; x <= 1; x++)
    {
        for (int z = 1; z >= -1; z--)
        {
            float2 gridCellTilt = float2(x, z) * lambda;
            
            float Wn = Wj[x + 1] * Wj[z + 1];
            
            gridCellTilt = float2(standardDeviationX * gridCellTilt.x, standardDeviationY * (gridCellTilt.x * correlationCoefficient + sqrt(1.0f - correlationCoefficient * correlationCoefficient) * gridCellTilt.y));
            
            gridCellTilt += totalSlope;
            
            float3 cellNormal = normalize(float3(-gridCellTilt.x, 1.0f, -gridCellTilt.y));
            
            float normalDotView = dot(cellNormal, viewDir);
            float normalDotMacronormal = dot(cellNormal, macronormal);
            
            float3 reflectedViewDir = normalize(reflect(-viewDir, cellNormal));
            
            float maskingShadowing = MaskingShadowing(viewDir, reflectedViewDir, totalSlope, covarianceMatrix);
            float fresnel = Fresnel(viewDir, cellNormal, covarianceMatrix);
            
            float J = 4.0f * abs(normalDotView) * pow(abs(normalDotMacronormal), 3);
            float alpha = J * A;
            
            environmentalColor += Wn * maskingShadowing * fresnel * max(0.0f, normalDotView) / max(normalDotMacronormal, 0.000001f) * SkyboxTexture.SampleLevel(LinearSampler, reflectedViewDir, alpha).xyz;
        }
    }
    
    environmentalColor = dot(normal, macronormal) / max(dot(normal, viewDir), 0.000001f) * environmentalColor;
    
    return environmentalColor;
}

float3 ScatteredLight(float3 lightDir, float3 viewDir, float3 normal, float height, float smithLight, float foam)
{
    height = max(0.0f, height);
    
    float heightValue = m_K1 * height * pow(max(0.0f, dot(lightDir, -viewDir)), 4);

    float sunAngleValue = pow(0.5f - 0.5f * dot(lightDir, normal), 3);
    
    float viewAngleValue = m_K2 * pow(max(0.0f, dot(viewDir, normal)), 2);
    
    float3 scatteredLight = ((heightValue * sunAngleValue + viewAngleValue) / (1.0f + smithLight)) * m_WaterScatterColor * m_LightColor;

    scatteredLight += (m_K3 * max(0.0f, dot(lightDir, normal))) * m_WaterScatterColor * m_LightColor;
    
    scatteredLight += (m_K4 * foam) * (m_AirBubblesColor * m_LightColor);
    
    return scatteredLight;
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
    
    float baseVariance = 0.00f;
    
    float xVariance = totalSecondOrderMoments.x - totalSlope.x * totalSlope.x + baseVariance;
    float yVariance = totalSecondOrderMoments.y - totalSlope.y * totalSlope.y + baseVariance;
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
    //float fresnel = pow(1.0 - max(0, dot(viewDir, normal)), 5);
    
    float3 specularColor = SpecularColor(D, maskingShadowingValue, fresnel, normal, macronormal, viewDir);
    //specularColor = specularColor / (1.0f + specularColor);
    
    float3 environmentalColor = EnvironmentalColor(covarianceMatrix, totalSlope, viewDir, normal, macronormal);
    //environmentalColor = environmentalColor / (1.0f + environmentalColor);
    
    float smithLight = Smith(-lightDir, totalSlope, covarianceMatrix);
    float3 scatteredLight = ScatteredLight(-lightDir, viewDir, normal, input.WorldPosition.y, smithLight, foam) * (1.0f - fresnel);
    
    float3 finalColor = specularColor + environmentalColor + scatteredLight;
    
    finalColor = finalColor / (1.0f + finalColor);
    
    //float3 ambientLight = m_AmbientLightIntensity * m_LightColor * 0.15f;
    //float3 diffuseLight = m_LightColor * input.Color * max(0, dot(normal, -lightDir));
    ////float3 specularLight = m_SpecularColor * pow(max(0, dot(halfVector, normal)), 32) * fresnel;
    
    //float3 reflectedViewDir = normalize(reflect(-viewDir, normal));
    //float3 environmentalReflection = SkyboxTexture.Sample(LinearSampler, reflectedViewDir).xyz * m_AmbientLightIntensity;// * fresnel;
    
    //finalColor = environmentalReflection;
    
    //float3 finalColor = ambientLight + diffuseLight + specularColor + environmentalColor;
    //finalColor = finalColor / (1.0f + finalColor);
    
    //float3 finalColor = ambientLight + diffuseLight * input.Color + specularLight * fresnel + environmentalReflection;
    
    //if (input.Position.x > 1920 * 0.45)
    //    finalColor = TessendorfLighting(normal, lightDir, viewDir, m_LightColor, input.WorldPosition, input.EyePos, m_Snell, m_kDiffuse);
    
    //float foam = saturate(slopeSample.a);
    
    finalColor = lerp(finalColor, m_FoamColor, foam); //clamp((m_FoamBias - slopeSample.a) / m_FoamBias, 0, 1));
    
    //finalColor += m_FogColor * saturate(length(input.WorldPosition - input.EyePos) / m_FogDistance);
    
    output.Color = float4(finalColor, 1.0f);
   
    return output;
}