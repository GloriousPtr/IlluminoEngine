static const float PI = 3.141592653589793;
static const float EPSILON = 1.17549435E-38;

struct VertexIn
{
	float4 Position : POSITION;
	float4 Normal : NORMAL;
	float4 Tangent : TANGENT;
	float4 Bitangent : BITANGENT;
	float2 UV : TEXCOORD;
};

struct VertexOut
{
	float4 CameraPosition : CAMERA_POSITION;
	float4 WorldPosition : WORLD_POSITION;
	float4 Position : SV_POSITION;
	float3x3 WorldNormal : WORLD_NORMAL;
	float3 Normal : NORMAL;
	float2 UV : TEXCOORD;
};

cbuffer Camera : register (b0)
{
	row_major float4x4 u_ViewProjection;
	float4 u_CameraPosition;
}

cbuffer Properties : register (b2)
{
	row_major float4x4 u_Model;
}

VertexOut VS_main(VertexIn v)
{
	VertexOut output;

	output.CameraPosition = u_CameraPosition;
	output.WorldPosition = mul(v.Position, u_Model);
	output.Position = mul(output.WorldPosition, u_ViewProjection);

	float3 T = normalize(mul(u_Model, v.Tangent).xyz);
	float3 B = normalize(mul(u_Model, v.Bitangent).xyz);
	output.Normal = normalize(mul(u_Model, v.Normal).xyz);
	output.WorldNormal = transpose(float3x3(T, B, output.Normal));

	output.UV = v.UV;

	return output;
}

struct PointLight
{
	float4 Position;

	/* rgb: color, a: intensity */
	float4 Color;

	/* packed into a vec4
	x: range
	y: cutOffAngle
	z: outerCutOffAngle
	w: unused */
	float4 Factors;
};

cbuffer LightData : register (b1)
{
	PointLight u_PointLights[3];
	int u_PointLightsSize;
}

cbuffer MaterialData : register (b3)
{
	float4 u_MRAO;
}

Texture2D u_Albedo : register(t0);
Texture2D u_NormalMap : register(t1);

SamplerState u_Sampler : register(s0);

// N: Normal, H: Halfway, a2: pow(roughness, 2)
float DistributionGGX(const float3 N, const float3 H, const float a2)
{
	float NdotH = max(dot(N, H), 0.0);
	float denom = mul(mul(NdotH, NdotH), (a2 - 1.0)) + 1.0;
	return a2 / mul(mul(PI, denom), denom);
}

// N: Normal, V: View, L: Light, k: (roughness + 1)^2 / 8.0
float GeometrySmith(const float NdotL, const float NdotV, const float k)
{
	float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
	float ggx2 = NdotL / (NdotL * (1.0 - k) + k);

	return ggx1 * ggx2;
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	float minusR = 1.0 - roughness;
	return F0 + (max(float3(minusR, minusR, minusR), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float LengthSq(const float3 v)
{
	return dot(v, v);
}

float4 PS_main(VertexOut input) : SV_TARGET
{
	float4 albedo = u_Albedo.Sample(u_Sampler, input.UV);
	if (albedo.a < 0.05)
		discard;

	float3 tangentNormal = u_NormalMap.Sample(u_Sampler, input.UV).rgb * 2.0 - 1.0;
	float3 normal = normalize(mul(input.WorldNormal, tangentNormal));
	float metalness = u_MRAO.r;
	float roughness = u_MRAO.g;

	float3 view = normalize(input.CameraPosition.xyz - input.WorldPosition.xyz);
	float NdotV = max(dot(normal, view), 0.0);

	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo.rgb, metalness);

	float a2 = roughness * roughness;
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;

	float3 Lo = float3(0.0, 0.0, 0.0);
	for (int i = 0; i < u_PointLightsSize; i++)
	{
		PointLight light = u_PointLights[i];
		float3 L = normalize(light.Position.xyz - input.WorldPosition.xyz);
		float NdotL = max(dot(normal, L), 0.0);
		float lightDistance2 = LengthSq(light.Position.xyz - input.WorldPosition.xyz);

		float lightRadius2 = light.Factors.x * light.Factors.x;
		float attenuation = saturate(1 - ((lightDistance2 * lightDistance2) / (lightRadius2 * lightRadius2)));
		attenuation = (attenuation * attenuation) / (lightDistance2 + 1.0);

		float3 radiance = light.Color.rgb * light.Color.a * attenuation;

		float3 H = normalize(L + view);
		float NDF = DistributionGGX(normal, H, a2);
		float G = GeometrySmith(NdotL, clamp(NdotV, 0.0, 1.0), k);
		float3 F = FresnelSchlickRoughness(clamp(dot(H, view), 0.0, 1.0), F0, roughness);

		float3 numerator = NDF * G * F;
		float denom = max(4.0 * NdotV * NdotL, 0.0001);
		float3 specular = numerator / denom;

		float3 kD = (1.0 - F) * (1.0 - metalness);
		Lo += (kD * (albedo.rgb / PI) + specular) * radiance * NdotL;
	}

	float ambient = 0.0f;

	float3 color = Lo + ambient;
	return float4(color, 1.0);
}
