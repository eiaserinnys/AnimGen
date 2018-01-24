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
	float2 txOfs = float2(1 / ShadowExtent.x, 1 / ShadowExtent.y);

	float bias = 0.0005;
	int range = 2;
	float shadow = 0;

	for (int i = -range; i <= range; ++i)
	{
		for (int j = -range; j <= range; ++j)
		{
			float shadowSample = txShadow.Sample(smShadow, lightTex + float2(j, i) * txOfs).x;
			shadow = shadow + (shadowSample + bias < lightSpaceDepth);
		}
	}

	shadow = 1 - shadow / (range * 2 + 1) / (range * 2 + 1);
	
	return float3(shadow, txShadow.Sample(smShadow, lightTex).x, lightSpaceDepth);
}

//------------------------------------------------------------------------------
float3 SampleNormal(float2 tex)
{
	float4 normal = txNormal.Sample(smNormal, tex);
	return normalize(normal.xyz * 2 - 1);
}

//------------------------------------------------------------------------------
float GetOutline(float3 vE, float3 vN, float2 tex, float depth)
{
	float2 txOfs = float2(1 / ViewportDesc.x, 1 / ViewportDesc.y);

	float4 depthN = float4(
		txDepth.Sample(smDepth, tex + float2(-txOfs.x, 0)).x,
		txDepth.Sample(smDepth, tex + float2(+txOfs.x, 0)).x,
		txDepth.Sample(smDepth, tex + float2(0, -txOfs.y)).x,
		txDepth.Sample(smDepth, tex + float2(0, +txOfs.y)).x);

	float depthBias = 0.25;
	float byDepth = saturate(
		max((depthN.x - depth) / depthBias, 0) +
		max((depthN.y - depth) / depthBias, 0) +
		max((depthN.z - depth) / depthBias, 0) +
		max((depthN.w - depth) / depthBias, 0));

	float3 n[4] = 
	{
		SampleNormal(tex + float2(-txOfs.x, 0)),
		SampleNormal(tex + float2(+txOfs.x, 0)),
		SampleNormal(tex + float2(0, -txOfs.y)),
		SampleNormal(tex + float2(0, +txOfs.y))
	};

	float bias = 0.1;
	n[0] = lerp(n[0], vN, abs(depthN.x - depth) / bias);
	n[1] = lerp(n[1], vN, abs(depthN.y - depth) / bias);
	n[2] = lerp(n[2], vN, abs(depthN.z - depth) / bias);
	n[3] = lerp(n[3], vN, abs(depthN.w - depth) / bias);

	float a = 10;
	float byNormal = saturate(
		//max((1 - dot(vN, n[0])) * a, 0) +
		max((1 - dot(vN, n[1])) * a, 0) +
		//max((1 - dot(vN, n[2])) * a, 0) +
		max((1 - dot(vN, n[3])) * a, 0));

	//return byNormal;
	//return byDepth;
	return saturate(byNormal * 0.75 + byDepth);
}

//------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 albedo = txAlbedo.Sample(smAlbedo, input.Tex);
	float depth = txDepth.Sample(smDepth, input.Tex).x;
	float3 vN = SampleNormal(input.Tex);

	// 원래 월드 공간으로 역 트랜스폼
	float4 orgPos = GetOriginalPosition(float3(input.Tex.xy, depth));
	float3 vE = normalize(EyePosition.xyz - orgPos.xyz);

	// 라이트 공간에서 텍스처 좌표를 계산
	float3 shadowResult = GetShadow(orgPos.xyz);
	float shadow = shadowResult.x;

	// 외곽선 계산
	float o = GetOutline(vE, vN, input.Tex, depth);

	// 라이팅
	float3 vL = -MainLightDir.xyz;
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
		(1 - o) * (albedo.xyz * lit + s) + 
		o * float3(1, 0.75, 0.25) * 0.05,
		(albedo.w > 0) + o);

	//final = float4(float3(o, o, o), 1);
	//final = float4(float3(l, l, l) + float3(o, o, o), albedo.w);
	//final = float4(vN.xyz, 1);
	//final = float4(shadow, shadow, shadow, 1);
	//final = float4(shadowResult.y, shadowResult.y, shadowResult.y, 1);
	//final = float4(lightSpaceDepth, 0, shadowDepth, 1);
	//final = float4(lightNdc.z, 0, 0, 1);
	//final = float4(shadowDepth, 0, 0, 1);
	//final = float4((lightTex.x + 1) / 2, (1 - lightTex.y) / 2, 0, 1);
	//final = float4((1 + lightNdc.x) / 2, (1 - lightNdc.y) / 2, 0, 1);

	return final;
}