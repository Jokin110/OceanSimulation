struct VSInput
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

struct VSOutput
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    output.Position = input.Position;
    output.Color = input.Color;
    
    return output;
}