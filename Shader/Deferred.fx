//--------------------------------------------------------------------------------------
cbuffer Constants : register(b0)
{
	float4 MainLightDir;
	float4 ViewportDesc;
	float4x4 InvWorldViewProj;
	float4x4 LightViewProj;
	float4 ShadowExtent;
	float4 EyePosition;
};

Texture2D		txAlbedo : register(t0);
SamplerState	smAlbedo : register(s0);

Texture2D		txNormal : register(t1);
SamplerState	smNormal : register(s1);

Texture2D		txView : register(t2);
SamplerState	smView : register(s2);

Texture2D		txDepth : register(t3);
SamplerState	smDepth : register(s3);

Texture2D		txShadow : register(t4);
SamplerState	smShadow : register(s4);

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

VS_OUTPUT VS(uint id : SV_VertexID)
{
	VS_OUTPUT Output;
	Output.Tex = float2((id << 1) & 2, id & 2);
	Output.Pos = float4(Output.Tex * float2(2, -2) + float2(-1, 1), 0.99, 1);
	return Output;
}

//------------------------------------------------------------------------------
float4 GetOriginalPosition(float3 uvw)
{
	float z = -uvw.z;
	float zn = ViewportDesc.z;
	float zf = ViewportDesc.w;
	float zR = zf / (zn - zf);

	float4 ndc = float4(
		(uvw.x - 0.5f) * 2,
		(0.5f - uvw.y) * 2,
		zR * (zn + z) / (-z),
		1);
	ndc = ndc * (-z);

	return mul(ndc, InvWorldViewProj);
}

//------------------------------------------------------------------------------
float3 GetShadow(float3 orgPos)
{
	float4 lightNdc = mul(float4(orgPos, 1), LightViewProj);
	float lightSpaceDepth = lightNdc.z / lightNdc.w;

	lightNdc = lightNdc / lightNdc.w;

	float2 lightTex = float2((lightNdc.x + 1) / 2, (1 - lightNdc.y) / 2);
	float shadowDepth = txShadow.Sample(smShadow, lightTex);

	float shadow = shadowDepth + 0.0001 < lightSpaceDepth;
	shadow = 1 - shadow;

	return float3(shadow, shadowDepth, lightSpaceDepth);
}

//------------------------------------------------------------------------------
float GetOutline(float2 tex, float depth)
{
	float2 txOfs = float2(1 / ViewportDesc.x, 1 / ViewportDesc.y);

	float4 depthN = float4(
		txDepth.Sample(smDepth, tex + float2(-txOfs.x, 0)).x,
		txDepth.Sample(smDepth, tex + float2(+txOfs.x, 0)).x,
		txDepth.Sample(smDepth, tex + float2(0, -txOfs.y)).x,
		txDepth.Sample(smDepth, tex + float2(0, +txOfs.y)).x);

	return saturate(
		depth - depthN.x +
		depth - depthN.y +
		depth - depthN.z +
		depth - depthN.w);
}

//------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 albedo = txAlbedo.Sample(smAlbedo, input.Tex);
	float4 normal = txNormal.Sample(smNormal, input.Tex);
	float depth = txDepth.Sample(smDepth, input.Tex).x;

	// 원래 월드 공간으로 역 트랜스폼
	float4 orgPos = GetOriginalPosition(float3(input.Tex.xy, depth));

	// 라이트 공간에서 텍스처 좌표를 계산
	float3 shadowResult = GetShadow(orgPos.xyz);
	float shadow = shadowResult.x;

	// 외곽선 계산
	float o = GetOutline(input.Tex, depth);

	// 라이팅
	float3 vL = -MainLightDir.xyz;
	float3 vE = normalize(EyePosition - orgPos.xyz);
	float3 vN = normalize(normal.xyz * 2 - 1);
	float3 vH = normalize(vL + vE);

	float l = saturate(dot(vL, vN)) * shadow;
	float s = saturate(pow(max(dot(vN, vH), 0), albedo.w * 255));

	float3 skylight = float3(33.0/255, 98.0/255, 202.0/255) * 0.75;

	float3 lit = l * (1 - skylight) * (1 / vL.y);

	float3 ambient = skylight;

	float mag = 1 / sqrt(3) * 4;
	float floorRef = 0.75;
	lit.xyz = lit.xyz + saturate(dot(vN, -normalize(float3(+1, -1, +1)))) * ambient / mag;
	lit.xyz = lit.xyz + saturate(dot(vN, -normalize(float3(+1, -1, -1)))) * ambient / mag;
	lit.xyz = lit.xyz + saturate(dot(vN, -normalize(float3(-1, -1, +1)))) * ambient / mag;
	lit.xyz = lit.xyz + saturate(dot(vN, -normalize(float3(-1, -1, -1)))) * ambient / mag;
	lit.xyz = lit.xyz + saturate(dot(vN, -normalize(float3(+1, +1, +1)))) * ambient / mag * floorRef;
	lit.xyz = lit.xyz + saturate(dot(vN, -normalize(float3(+1, +1, -1)))) * ambient / mag * floorRef;
	lit.xyz = lit.xyz + saturate(dot(vN, -normalize(float3(-1, +1, +1)))) * ambient / mag * floorRef;
	lit.xyz = lit.xyz + saturate(dot(vN, -normalize(float3(-1, +1, -1)))) * ambient / mag * floorRef;

	float4 final;
	final = float4(
		albedo.xyz * lit + 
		s + 
		float3(o, o, o), 
		(albedo.w + o) > 0);

	//final = float4(float3(l, l, l) + float3(o, o, o), albedo.w);
	//final = float4(normal.xyz + float3(o, o, o), albedo.w);
	//final = float4(shadow, shadow, shadow, 1);
	//final = float4(shadowResult.y, shadowResult.y, shadowResult.y, 1);
	//final = float4(lightSpaceDepth, 0, shadowDepth, 1);
	//final = float4(lightNdc.z, 0, 0, 1);
	//final = float4(shadowDepth, 0, 0, 1);
	//final = float4((lightTex.x + 1) / 2, (1 - lightTex.y) / 2, 0, 1);
	//final = float4((1 + lightNdc.x) / 2, (1 - lightNdc.y) / 2, 0, 1);

	return final;
}