
struct PSInput
{
    float3 worldPosition : TEXCOORD0;
    float3 normalWS : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float3 eyePos : TEXCOORD3;
    float jacobian : JACOBIAN;
    float3 color : COLOR;
    float4 position : SV_Position;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

float3 TessendorfLighting(float3 normal, float3 lightDir, float3 viewDir, float3 skyColor, float3 P, float3 E, float nSnell = 1.33, float kDiffuse = 0.0)
{
    float3 upwelling = float3(0.1, 0.3, 0.4);
    float3 air = float3(0.1, 0.1, 0.1);
    
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
    
    float3 color = dist * (reflectivity * skyColor + (1 - reflectivity) * upwelling) + (1 - dist) * air;
        
    return color;
}

PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
    
    float3 lightColor = float3(0.53f, 0.81f, 0.92f);
    float3 lightDir = normalize(float3(0, -0.5, -1));
    float foamBias = 0.3;
    float3 foamColor = float3(1, 1, 1);
    
    float3 normal = normalize(input.normalWS);
    float3 viewDir = normalize(input.viewVector);
    float3 halfVector = normalize(-lightDir + viewDir);
    
    float3 ambientLight = 0.25 * lightColor;
    float3 diffuseLight = lightColor * max(0, dot(normal, -lightDir));
    float3 specularLight = float3(1, 1, 1) * pow(max(0, dot(halfVector, normal)), 32);
    float fresnel = pow(1.0 - max(0, dot(viewDir, normal)), 5);
    
    float3 finalColor = ambientLight + diffuseLight * input.color + specularLight * fresnel;
    
    if (input.position.x > 1920 * 0.45)
        finalColor = TessendorfLighting(normal, lightDir, viewDir, lightColor, input.worldPosition, input.eyePos, 1.33, 0.01);
    
    finalColor = lerp(finalColor, foamColor, clamp((foamBias - input.jacobian) / foamBias, 0, 1));
    
    output.color = float4(finalColor, 1.0);
    
    //output.color = float4(input.color, 1.0);
    
    return output;
}