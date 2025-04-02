#include "ShaderStructs.hlsli"
#include "Lighting.hlsli"
#define MAX_LIGHTS 5

cbuffer ExternalData : register(b0)
{
    float3 colorTint;
    float2 uvScale;
    float2 uvOffset;
    float roughness;
    float3 cameraPos;
    float3 ambientColor;
    int lightCount;
    Light lights[MAX_LIGHTS];
}

//texture and sampler resouces
Texture2D SurfaceTexture  : register(t0); // "t" registers for textures
Texture2D NormalMap       : register(t1); // "t" registers for textures
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
	//normalize incoming normals
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    
    input.uv = input.uv * uvScale + uvOffset;
    
    float3 N = input.normal;
    float3 T = input.tangent;
    T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    
    
    
    
    float3 unpackedNormal = normalize(NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1);
    
    // Assumes that input.normal is the normal later in the shader
    input.normal = mul(unpackedNormal, TBN); // Note multiplication order!
    
    float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
    surfaceColor *= colorTint;
    
    float3 totalLight = ambientColor * surfaceColor;

     //looping through lights instead of one light
    for (int i = 0; i < lightCount; i++)
    {
		
        //set and normalize the light varaible in the loop
        Light light = lights[i];
        light.Direction = normalize(light.Direction);
		
		//switch statement for lights[i].Type
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                totalLight += DirLight(light, input.normal, input.worldPos, cameraPos, roughness, surfaceColor);
                break;

            case LIGHT_TYPE_POINT:
                totalLight += PointLight(light, input.normal, input.worldPos, cameraPos, roughness, surfaceColor);
                break;

            case LIGHT_TYPE_SPOT:
                totalLight += SpotLight(light, input.normal, input.worldPos, cameraPos, roughness, surfaceColor);
                break;
        }
    }
    return float4(totalLight, 1);
}