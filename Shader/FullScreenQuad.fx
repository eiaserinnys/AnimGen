Texture2D txDiffuse : register(t0);
SamplerState samPoint : register(s0);

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
	return txDiffuse.Sample(samPoint, input.Tex);
}