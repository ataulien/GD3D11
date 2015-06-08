#include<VS_ParticlePoint.hlsl>

cbuffer ParticleGSInfo : register( b2 )
{
	float3 CameraPosition;
    float pad;
};

struct PS_INPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vNormalVS		: TEXCOORD4;
	float3 vViewPosition	: TEXCOORD5;
	float4 vPosition		: SV_POSITION;
};

[maxvertexcount(4)]
void GSMain(point VS_OUTPUT input[1], inout TriangleStream<PS_INPUT> OutputStream)
{
    float3 planeNormal = input[0].vPosition - CameraPosition;
    //planeNormal.y = 0.0f; // For tree bilboard
    planeNormal = normalize(-planeNormal);
    
    float3 upVector;
    float3 rightVector;
   
    
    
    //input[0].vSize *= 0.5f;
    
    //rightVector = rightVector * 100.0f;
    //upVector *= 100.0f;
    
    // Construct vertices
    // We get the points by using the billboards right vector and the billboards height
    float3 vert[4];
    
    if(input[0].type == 3)
    {
		// Make up/right vectors along the velocity-vector
        float3 velYPos		= normalize(input[0].vVelocity);
		float3 velXPos	    = normalize(cross(planeNormal, velYPos));
          
        //velYPos = normalize(cross(planeNormal, velXPos)); 

        rightVector = velXPos;
        upVector = velYPos;
    }else if(input[0].type == 2)
	{
		// xz-plane
		upVector = float3(0.0f, 0.0f, 1.0f);
		rightVector = float3(1.0f,0.0f,0.0f); // FIXME: Maybe rotate this with the vob?
	}else
    {
        // Construct up and right vectors
        upVector = float3(0.0f, 1.0f, 0.0f);
        rightVector = normalize(cross(planeNormal, upVector));
        
        // Construct better up-vector   
        upVector = normalize(cross(planeNormal, rightVector)); 
    }
	
	if(input[0].type == 5)
	{
		//upVector = float3(0.0f, 1.0f, 0.0f);
		//rightVector = float3(1.0f,0.0f,0.0f); // FIXME: Maybe rotate this with the vob?
		
		// Scale vectors
		rightVector *= input[0].vSize.x;
		upVector *= input[0].vSize.y;
	}else
	{
		// Scale vectors
		rightVector *= input[0].vSize.x;
		upVector *= input[0].vSize.y;
    }
	
    vert[0] = input[0].vPosition - rightVector - upVector; // Get bottom left vertex
    vert[1] = input[0].vPosition + rightVector - upVector; // Get bottom right vertex
    vert[2] = input[0].vPosition - rightVector + upVector; // Get top left vertex
    vert[3] = input[0].vPosition + rightVector + upVector; // Get top right vertex
    
    // Get billboards texture coordinates
    float2 texCoord[4];
    texCoord[0] = float2(0, 1);
    texCoord[1] = float2(1, 1);
    texCoord[2] = float2(0, 0);
    texCoord[3] = float2(1, 0);
    
    // Append the two triangles to the stream
    
    PS_INPUT outputVert = (PS_INPUT)0;
    for(int i = 0; i < 4; i++)
    {
        outputVert.vPosition = mul(float4(vert[i], 1.0f), M_ViewProj);
        outputVert.vTexcoord = texCoord[i];
        outputVert.vDiffuse = float4(input[0].vDiffuse.rgb, pow(input[0].vDiffuse.a, 2.2f));

        OutputStream.Append(outputVert);
    }
}