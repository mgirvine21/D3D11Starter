#include "ShaderStructs.hlsli"
#include "Lighting.hlsli"

#define NUM_LIGHTS 5

cbuffer ExternalData : register(b0)
{
	// Scene related
    Light lights[NUM_LIGHTS];
    float3 ambientColor;

	// Camera related
    float3 cameraPosition;

	// Material related
    float3 colorTint;
    float roughness;
    float2 uvScale;
    float2 uvOffset;
    int useSpecularMap;
	
}

//texture and sampler resouces
Texture2D SurfaceTexture : register(t0); // "t" registers for textures
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
	// Clean up un-normalized normals
    input.normal = normalize(input.normal);

	// Adjust the uv coords
    input.uv = input.uv * uvScale + uvOffset;

	// Sample the texture and tint for the final surface color
    float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
    surfaceColor *= colorTint;
 
	// Start off with ambient
    float3 totalLight = ambientColor * surfaceColor;
	
	// Loop and handle all lights
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
		// Grab this light and normalize the direction (just in case)
        Light light = lights[i];
        light.Direction = normalize(light.Direction);

		// Run the correct lighting calculation based on the light's type
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                totalLight += DirLight(light, input.normal, input.worldPos, cameraPosition, roughness, surfaceColor);
                break;

            case LIGHT_TYPE_POINT:
                totalLight += PointLight(light, input.normal, input.worldPos, cameraPosition, roughness, surfaceColor);
                break;

            case LIGHT_TYPE_SPOT:
                totalLight += SpotLight(light, input.normal, input.worldPos, cameraPosition, roughness, surfaceColor);
                break;
        }
    }

	// Should have the complete light contribution at this point
    return float4(totalLight, 1);
}