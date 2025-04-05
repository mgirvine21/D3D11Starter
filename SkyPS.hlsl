#include "ShaderStructs.hlsli"

TextureCube SkyTexture : register(t0);
SamplerState BasicSampler : register(s0);

float4 main(VertexToPixel_Sky input) : SV_TARGET
{
    //sample cube map given direction from Sky VS
    return SkyTexture.Sample(BasicSampler, input.sampleDir);
}