#define MAX_NUM_LIGHTS_PER_TILE 100
#define NUM_THREADS_X 10
#define NUM_THREADS_Y 10

//all the lights
Buffer<float4> pointLightBuffer : register(t0);
//Buffer<float4> spotLightBuffer : register(t1);

//the depth values from predepthpass
texture2D<float> depthTexture : register(t2);

//I will write all the lights for that tile in this buffer, which the pixel shader will use to index into all of the lights
RWBuffer<uint> lightIndexBufferOut : register(u0);

// HELPER FUNCTIONS //
float3 CreatePlaneNormal(float3 p0, float3 p1)
{
	return normalize(cross(p0, p1));
}

[numthreads(1, 1, 1)] //i need to make sure I use this correctly so I'm setting it to one for now
void main(uint index : SV_GroupIndex)
{
	//so I will use the depthTexture to see if the light should even be lit
	//for example, if the light is behind the object, I don't want to light it

	
}