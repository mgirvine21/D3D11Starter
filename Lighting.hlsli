#ifndef __GGP_LIGHTING__
#define __GGP_LIGHTING__



#define MAX_SPECULAR_EXPONENT 256.0f

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2



struct Light
{
    int Type; //kind of light 0 / 1 / 2
    float3 Direction; //for direction and spotlight
    float range; //for point and spot light
    float3 Position; //for post and spot light
    float Intensity; //all lights
    float3 Color; //all lights
    float SpotInnerAngle; //full light
    float SpotOuterAngle; //no lihgt outside this
    float2 Padding; //16 pyte boundary
};

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
    return att * att;
}

//helper lighting functions
float Diffuse(float3 normal, float3 toLight)
{
    return saturate(dot(normal, toLight));
}

float SpecularPhong(float3 normal, float3 dirToLight, float3 toCamera, float roughness)
{
    //reflection of light direction
    //direction the light is travelling
    float3 reflection = reflect(-dirToLight, normal);
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    return roughness == 1 ? 0.0f : pow(max(dot(toCamera, reflection), 0.0f), specExponent);
    
}

float3 DirLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor)
{
	// Get normalize direction to the light
    float3 toLight = normalize(-light.Direction);
    float3 toCamera = normalize(camPos - worldPos);

	// Calculate the light amounts
    float diff = Diffuse(normal, toLight);
    float spec = SpecularPhong(normal, toLight, toCamera, roughness);
    
    // Cut the specular if the diffuse contribution is zero
    // - any() returns 1 if any component of the param is non-zero
    // - In other words:
    // - If the diffuse amount is 0, any(diffuse) returns 0
    // - If the diffuse amount is != 0, any(diffuse) returns 1
    // - So when diffuse is 0, specular becomes 0
    spec *= any(diff);


	// Combine
    //tint specular for fun
    return ((diff + spec) * surfaceColor) * light.Intensity * light.Color;
    //return (diff * surfaceColor + spec) * light.Intensity * light.Color;
}


float3 PointLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor)
{
    //direction from pixle  to the light
    float3 toLight = normalize(light.Position - worldPos);
    //View Vector from this pixle to the camera
    float3 toCamera = normalize(camPos - worldPos);

	// Calculate the light amounts
    float atten = Attenuate(light, worldPos);
    float diff = Diffuse(normal, toLight);
    float spec = SpecularPhong(normal, toLight, toCamera, roughness);
    
    spec *= any(diff);

	// Combine
    return (diff * surfaceColor + spec) * atten * light.Intensity * light.Color;
}


float3 SpotLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor)
{
	// Calculate the spot falloff
    float3 toLight = normalize(light.Position - worldPos);
    float pixelAngle = saturate(dot(-toLight, light.Direction));

    float cosOuter = cos(light.SpotOuterAngle);
    float cosInner = cos(light.SpotInnerAngle);
    float falloffRange = cosOuter - cosInner;

    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);

	// Combine with the point light calculation
	// Note: This could be optimized a bit!  Doing a lot of the same work twice!
    return PointLight(light, normal, worldPos, camPos, roughness, surfaceColor) * spotTerm;
}
#endif