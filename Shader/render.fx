//--------------------------------------------------------------------------------------
// File: render.fx
//
// Alexis LAFONT - 2014
//--------------------------------------------------------------------------------------

//*********************************//
//			Constant buffer		   //
//*********************************//

cbuffer CbVS : register (b1)
{
	float 		c_TexSideSize;
};

cbuffer CbGS : register (b2)
{
	float4x4 	c_View;
	float4x4 	c_Proj;
};

static const float4 g_positions[6] =
{
	float4( -10, -10, 0, 1 ),
	float4( -10,  10, 0, 1 ),
	float4(  10,  10, 0, 1 ),
			     
	float4(  10,  10, 0, 1 ),
	float4(  10, -10, 0, 1 ),
	float4( -10, -10, 0, 1 )
};

static const float2 g_texcoords[6] =
{
	float2( 0, 1 ),
	float2( 0, 0 ),
	float2( 1, 0 ),

	float2( 1, 0 ),
	float2( 1, 1 ),
	float2( 0, 1 )
};

static const float2 g_texcoordsTri[3] =
{
	float2( 0, 2 ),
	float2( 0, 0 ),
	float2( 2, 0 ),
};

//*********************************//
//			VS/GS/PS In/Out		   //
//*********************************//

struct VS_INPUT
{
    float4   COLOR   	: COLOR;             //particle color
    uint	 ID			: SV_VERTEXID;       //sys-gen vertex id
};

struct VS_OUTPUT
{
    float4 Pos : COLOR;						// Pass the position (float3) with the weight (float) into color (float4)
};

struct GS_OUTPUT
{
    float4 Pos 		: SV_POSITION;
	float4 Color	: COLOR;
    float2 Tex 		: TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos 		: SV_POSITION;
    float4 Color 	: COLOR;
    float2 Tex 		: TEXCOORD0;
};

//*********************************//
//			  Resources			   //
//*********************************//

Texture2D 		m_NewPositionTex;
Texture2D		m_ParticleTex	: register( t1 );

SamplerState 	m_SimpleSamplerPS;


//*********************************//
//			Vertex Shader		   //
//*********************************//
VS_OUTPUT VS( VS_INPUT input )
{
    VS_OUTPUT output;
    
	int3 aTexCoord; // Look up the particle position
	
	aTexCoord.x = input.ID % c_TexSideSize;	// X
	aTexCoord.y = input.ID / c_TexSideSize;	// Y
	aTexCoord.z = 0;						// Mipmap

	output.Pos = m_NewPositionTex.Load(aTexCoord);

    return output;
}

//*********************************//
//			Geometry Shader		   //
//*********************************//
[ maxvertexcount( 6 ) ]
void GS( point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> SpriteStream )
{
	GS_OUTPUT output;
	
	// Rectangle
	
	float3 aRightVec = float3( -c_View._11, -c_View._21, -c_View._31 );
	float3 aUpVec = float3( c_View._12, c_View._22, c_View._32 );
	float4x4 VP = mul( c_Proj, c_View );

	float4 aQuad[6];


	aQuad[0] = float4( aRightVec - aUpVec, 0.0f);
	aQuad[1] = float4( aRightVec + aUpVec, 0.0f);
	aQuad[2] = float4( -aRightVec + aUpVec, 0.0f);

	aQuad[3] = float4( -aRightVec + aUpVec, 0.0f);
	aQuad[4] = float4( -aRightVec - aUpVec, 0.0f);
	aQuad[5] = float4( aRightVec - aUpVec, 0.0f);

	// First triangle
	for( int i = 0; i < 3; i++ )
	{
		float4x4 VP = mul( c_Proj, c_View );
			output.Pos = mul( ( input[0].Pos ).xyz, c_View ) + ( aQuad[i] * input[0].Pos.w );

		output.Color = float4( 0.2, 0.5, 0.3, 1.0 );
		output.Tex = g_texcoords[i];

		SpriteStream.Append( output );
	}

	SpriteStream.RestartStrip( );

	// Second triangle
	for( i = 3; i < 6; i++ )
	{
		float4x4 VP = mul( c_Proj, c_View );
			output.Pos = mul( ( input[0].Pos ).xyz, c_View ) + ( aQuad[i] * input[0].Pos.w );

		output.Color = float4( 0.2, 0.5, 0.3, 1.0 );
		output.Tex = g_texcoords[i];

		SpriteStream.Append( output );
	}
	

	// Triangle
	/*
	float3 aRightVec = float3( -c_View._11, -c_View._21, -c_View._31 );
	float3 aUpVec = float3( c_View._12, c_View._22, c_View._32 );
	float4x4 VP = mul( c_Proj, c_View );

	float4 aQuad[6];


	aQuad[0] = float4( aRightVec - aUpVec, 0.0f );
	aQuad[1] = float4( aRightVec + aUpVec, 0.0f );
	aQuad[2] = float4( -aRightVec + aUpVec, 0.0f );

	aQuad[3] = float4( -aRightVec + aUpVec, 0.0f );
	aQuad[4] = float4( -aRightVec - aUpVec, 0.0f );
	aQuad[5] = float4( aRightVec - aUpVec, 0.0f );

	// First triangle
	for( int i = 0; i < 3; i++ )
	{
		float4x4 VP = mul( c_Proj, c_View );
			output.Pos = mul( ( input[0].Pos ).xyz, c_View ) + ( aQuad[i] * input[0].Pos.w );

		output.Color = float4( 0.2, 0.5, 0.3, 1.0 );
		output.Tex = g_texcoordsTri[i];

		SpriteStream.Append( output );
	}

	SpriteStream.RestartStrip( );*/
}

//*********************************//
//			Pixel Shader		   //
//*********************************//
float4 PS( PS_INPUT input ) : SV_Target
{
	return m_ParticleTex.Sample( m_SimpleSamplerPS, input.Tex );
}