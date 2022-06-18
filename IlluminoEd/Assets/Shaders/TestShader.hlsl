struct VertexIn
{
	float4 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VertexOut
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer Properties : register (b1)
{
	row_major float4x4 u_MVP;
	float4 u_Color;
}

VertexOut VS_main(VertexIn v)
{
	VertexOut output;

	output.position = mul(v.position, u_MVP);
	output.uv = v.uv;

	return output;
}

struct PointLight
{
	float4 LightPosition;
	float4 LightColor;
	float4 LightFactors;
};

struct LightData
{
	int pointLightsSize;
	PointLight pointLights[3];
} u_LightData : register(u0);

Texture2D u_Albedo : register(t0);
Texture2D u_NormalMap : register(t1);

SamplerState u_Sampler : register(s0);

float4 PS_main(VertexOut i) : SV_TARGET
{
	float4 albedo = u_Albedo.Sample(u_Sampler, i.uv);
	if (albedo.a < 0.05)
		discard;

	float4 normal = u_NormalMap.Sample(u_Sampler, i.uv);

	float ambient = 1.0f;

	float4 Lo = u_LightData.pointLights[0].LightColor;

	float3 color = albedo.rgb * ambient * (Lo.rgb + float3(1, 1, 1));
	return float4(color, normal.a);
}
