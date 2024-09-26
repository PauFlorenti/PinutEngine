#extension GL_GOOGLE_include_directive : enable

// CONSTS ----------------------
const float PI = 3.1415926535897932384626433832795;

// STRUCTS --------------------

// FUNCTIONS --------------------------------------------------
// Polynomial approximation by Christophe Schlick
// Trowbridge-Reitz GGX - Normal Distribution Function
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
  float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return a2 / denom;
}

// Geometry Function
float GeometrySchlickGGX(float NdotV, float roughness)
{
  float r = roughness + 1.0;
  float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom/denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx1 = GeometrySchlickGGX(NdotV, roughness);
	float ggx2 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

// Fresnel Equation
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 computeAmbient(vec3 N, vec3 V, vec3 albedo, vec3 irradiance, vec3 F0)
{
    vec3 kS = FresnelSchlick(clamp(dot(N, V), 0.0, 1.0), F0);
    vec3 kD = 1.0 - kS;
    vec3 diffuse = albedo * irradiance;
    return (kD * diffuse);
}