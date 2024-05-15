struct VSIn
{
    float3 pos : POSITION;
};

struct VSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

cbuffer CB : register(b0)
{
    float Luminance;
}

VSOut VS_main(VSIn input, uint uVertexID : SV_VertexID)
{
    VSOut output = (VSOut) 0;
    output.pos = float4(input.pos, 1.0f);
    output.color = float4(Luminance.xxx * (uVertexID / 2.0f), 1.0f);

    return output;
}

float4 PS_main(VSOut input) : SV_TARGET0
{
    return input.color;
}