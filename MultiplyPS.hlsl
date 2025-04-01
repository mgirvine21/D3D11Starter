#include "ShaderStructs.hlsli"


cbuffer ExternalData : register(b0)
{
    float3 colorTint;
    float2 uvScale;
    float2 uvOffset;
}

//texture and sampler resouces
Texture2D SurfaceTextureStone : register(t0); // "t" registers for textures 1
Texture2D SurfaceTextureCobble : register(t1); // "t" registers for textures 2
SamplerState BasicSampler : register(s0); // "s" registers for samplers

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	
	//sampling
    input.uv = input.uv * uvScale + uvOffset;
    float3 surfaceColor = (SurfaceTextureStone.Sample(BasicSampler, input.uv).rgb) * (SurfaceTextureCobble.Sample(BasicSampler, input.uv).rgb);
	//colorTinting
    surfaceColor *= colorTint;
	
	
    return float4(surfaceColor, 1);
}