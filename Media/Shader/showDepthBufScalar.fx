float4 VSMain(uint vert : SV_VertexID) : SV_Position
{
		float x = (vert & 1) * 4.0f - 1.0f;
		float y = (vert >> 1) * 4.0f - 1.0f;
		return float4(x, y, 0.5f, 1.0f);
}

Texture2D tex : register(t0);

float4 PSMain(float4 pos : SV_Position) : SV_Target
{
		int x = (int) pos.x;
		int y = (int) pos.y;
				
		return tex.Load(int3(x, y, 0));
}