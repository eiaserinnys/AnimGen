//--------------------------------------------------------------------------------------
cbuffer Constants : register(b0)
{
	float2 RenderTargetExtent;
};

Texture2D		txAlbedo : register(t0);
SamplerState	smAlbedo : register(s0);

Texture2D		txNormal : register(t1);
SamplerState	smNormal : register(s1);

Texture2D		txView : register(t2);
SamplerState	smView : register(s2);

Texture2D		txDepth : register(t3);
SamplerState	smDepth : register(s3);

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

float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 albedo = txAlbedo.Sample(smAlbedo, input.Tex);
	float4 normal = txNormal.Sample(smNormal, input.Tex);

	float2 txOfs = float2(1 / RenderTargetExtent.x, 1 / RenderTargetExtent.y);

	float depth = txDepth.Sample(smDepth, input.Tex).x;
	float4 depthN = float4(
		txDepth.Sample(smDepth, input.Tex + float2(-txOfs.x, 0)).x,
		txDepth.Sample(smDepth, input.Tex + float2(+txOfs.x, 0)).x,
		txDepth.Sample(smDepth, input.Tex + float2(0, -txOfs.y)).x,
		txDepth.Sample(smDepth, input.Tex + float2(0, +txOfs.y)).x);

	float o = 
		depth - depthN.x +
		depth - depthN.y +
		depth - depthN.z +
		depth - depthN.w;
	o = clamp(o, 0, 1);

	//float4 final = float4(albedo.xyz + float3(o, o, o), albedo.w);
	float4 final = float4(normal.xyz + float3(o, o, o), albedo.w);

	return final;
}