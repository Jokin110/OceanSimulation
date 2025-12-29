
struct PSInput
{
    float3 worldPosition : TEXCOORD0;
    float3 normalWS : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float3 color : COLOR;
    float4 position : SV_Position;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
    
    float3 lightColor = float3(0.5, 0.8, 0.8);
    float3 lightDir = normalize(float3(0, -1, -1));
    
    float3 ambientLight = 0.1 * lightColor;
    float3 diffuseLight = lightColor * max(0, dot(normalize(input.normalWS), -lightDir));
    float3 specularLight = float3(1, 1, 1) * pow(max(0, dot(normalize(-lightDir + input.viewVector), normalize(input.normalWS))), 128);
    float3 rimLight = lightColor * pow(1.0 - max(0, dot(normalize(input.viewVector), normalize(input.normalWS))), 2);
    
    float3 finalColor = (ambientLight + diffuseLight + rimLight) * input.color + specularLight;
    
    output.color = float4(finalColor, 1.0);
    
    //output.color = float4(input.color, 1.0);
    
    return output;
}