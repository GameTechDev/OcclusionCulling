float4 VSMain(uint vert : SV_VertexID) : SV_Position
{
		float x = (vert & 1) * 4.0f - 1.0f;
		float y = (vert >> 1) * 4.0f - 1.0f;
		return float4(x, y, 0.5f, 1.0f);
}

Texture2D tex : register(t0);

float4 PSMain(float4 pos : SV_Position) : SV_Target
{
		int inX = (int) pos.x;
		int inY = (int) pos.y;

		// perform swizzle
		//int x = inX + (inX & ~3) + ((inY & 1) << 2);
		int x = (inX & 3) + ((inY & 1) << 2) + ((inX & ~3) << 1);
		int y = inY >> 1;
		
		return tex.Load(int3(x, y, 0));
}