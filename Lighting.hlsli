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

// CONSTANTS ===================
// A constant Fresnel value for non-metals (glass and plastic have values of about 0.04)
static const float F0_NON_METAL = 0.04f;

// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal

// Handy to have this as a constant
static const float PI = 3.14159265359f;


// PBR FUNCTIONS ================

// Lambert diffuse BRDF - Same as the basic lighting diffuse calculation!
// - NOTE: this function assumes the vectors are already NORMALIZED!
float DiffusePBR(float3 normal, float3 dirToLight)
{
    return saturate(dot(normal, dirToLight));
}



// Calculates diffuse amount based on energy conservation
//
// diffuse   - Diffuse amount
// F         - Fresnel result from microfacet BRDF
// metalness - surface metalness amount 
float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}
 


// Normal Distribution Function: GGX (Trowbridge-Reitz)
//
// a - Roughness
// h - Half vector
// n - Normal
// 
// D(h, n, a) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float D_GGX(float3 n, float3 h, float roughness)
{
	// Pre-calculations
    float NdotH = saturate(dot(n, h));
    float NdotH2 = NdotH * NdotH;
    float a = roughness * roughness;
    float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

	// ((n dot h)^2 * (a^2 - 1) + 1)
	// Can go to zero if roughness is 0 and NdotH is 1
    float denomToSquare = NdotH2 * (a2 - 1) + 1;

	// Final value
    return a2 / (PI * denomToSquare * denomToSquare);
}



// Fresnel term - Schlick approx.
// 
// v - View vector
// h - Half vector
// f0 - Value when l = n
//
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 F_Schlick(float3 v, float3 h, float3 f0)
{
	// Pre-calculations
    float VdotH = saturate(dot(v, h));

	// Final value
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}



// Geometric Shadowing - Schlick-GGX
// - k is remapped to a / 2, roughness remapped to (r+1)/2 before squaring!
//
// n - Normal
// v - View vector
//
// G_Schlick(n,v,a) = (n dot v) / ((n dot v) * (1 - k) * k)
//
// Full G(n,v,l,a) term = G_SchlickGGX(n,v,a) * G_SchlickGGX(n,l,a)
float G_SchlickGGX(float3 n, float3 v, float roughness)
{
	// End result of remapping:
    float k = pow(roughness + 1, 2) / 8.0f;
    float NdotV = saturate(dot(n, v));

	// Final value
	// Note: Numerator should be NdotV (or NdotL depending on parameters).
	// However, these are also in the BRDF's denominator, so they'll cancel!
	// We're leaving them out here AND in the BRDF function as the
	// dot products can get VERY small and cause rounding errors.
    return 1 / (NdotV * (1 - k) + k);
}


 
// Cook-Torrance Microfacet BRDF (Specular)
//
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - parts of the denominator are canceled out by numerator (see below)
//
// D() - Normal Distribution Function - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 f0, out float3 F_out)
{
	// Other vectors
    float3 h = normalize(v + l);

	// Run numerator functions
    float D = D_GGX(n, h, roughness);
    float3 F = F_Schlick(v, h, f0);
    float G = G_SchlickGGX(n, v, roughness) * G_SchlickGGX(n, l, roughness);
	
	// Pass F out of the function for diffuse balance
    F_out = F;

	// Final specular formula
	// Note: The denominator SHOULD contain (NdotV)(NdotL), but they'd be
	// canceled out by our G() term.  As such, they have been removed
	// from BOTH places to prevent floating point rounding errors.
    float3 specularResult = (D * F * G) / 4;

	// One last non-obvious requirement: According to the rendering equation,
	// specular must have the same NdotL applied as diffuse!  We'll apply
	// that here so that minimal changes are required elsewhere.
    return specularResult * max(dot(n, l), 0);
}




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

float3 DirLightPBR(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float metalness, float3 surfaceColor, float3 specularColor)
{
	// Get normalize direction to the light
    float3 toLight = normalize(-light.Direction);
    float3 toCamera = normalize(camPos - worldPos);

	// Calculate the light amounts
    float diff = DiffusePBR(normal, toLight);
    float3 F_out;
    float3 spec = MicrofacetBRDF(normal, toLight, toCamera, roughness, specularColor, F_out);
    
    // Calculate diffuse with energy conservation
	// (Reflected light doesn't get diffused)
    float3 balancedDiff = DiffuseEnergyConserve(diff, spec, metalness);

	// Combine amount with 
    return (balancedDiff * surfaceColor + spec) * light.Intensity * light.Color;
}


float3 PointLightPBR(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float metalness, float3 surfaceColor, float3 specularColor)
{
    //direction from pixle  to the light
    float3 toLight = normalize(light.Position - worldPos);
    //View Vector from this pixle to the camera
    float3 toCamera = normalize(camPos - worldPos);

	// Calculate the light amounts
    float atten = Attenuate(light, worldPos);
    float diff = DiffusePBR(normal, toLight);
    float3 F_out;
    float3 spec = MicrofacetBRDF(normal, toLight, toCamera, roughness, specularColor, F_out);
    
    // Calculate diffuse with energy conservation, including cutting diffuse for metals
    float3 balancedDiff = DiffuseEnergyConserve(diff, F_out, metalness);
    
    // Combine the final diffuse and specular values for this light
    return (balancedDiff * surfaceColor + spec) * light.Intensity * light.Color;
}


float3 SpotLightPBR(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float metalness, float3 surfaceColor, float3 specularColor)
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
    return PointLightPBR(light, normal, worldPos, camPos, roughness, metalness, surfaceColor, specularColor) * spotTerm;
}

// === LIGHT TYPES FOR BASIC LIGHTING ===============================


    float3 DirLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor)
    {
	    // Get normalize direction to the light
        float3 toLight = normalize(-light.Direction);
        float3 toCamera = normalize(camPos - worldPos);

	    // Calculate the light amounts
        float diff = Diffuse(normal, toLight);
        float spec = SpecularPhong(normal, toLight, toCamera, roughness);

	    // Combine
        return (diff * surfaceColor + spec) * light.Intensity * light.Color;
    }


    float3 PointLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor)
    {
	    // Calc light direction
        float3 toLight = normalize(light.Position - worldPos);
        float3 toCamera = normalize(camPos - worldPos);

	    // Calculate the light amounts
        float atten = Attenuate(light, worldPos);
        float diff = Diffuse(normal, toLight);
        float spec = SpecularPhong(normal, toLight, toCamera, roughness);

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
	    // Note: This could be optimized a bit
        return PointLight(light, normal, worldPos, camPos, roughness, surfaceColor) * spotTerm;
    }

#endif