//--------------------------------------------------------------------------------------
// File: Impulsion.fx
//
// Alexis LAFONT - 2014
//--------------------------------------------------------------------------------------

//*********************************//
//			Constant buffer		   //
//*********************************//

cbuffer CbFS : register (b0)
{
	float2 c_PointImp;
	float c_ForceImp;
	float c_RenderTime;
};

static const float3 g_positions[6] =
{
	float3(-1, -1, 0),
	float3(-1, 1, 0),
	float3(1, 1, 0),

	float3(1, 1, 0),
	float3(1, -1, 0),
	float3(-1, -1, 0)
};

static const float2 g_texcoords[6] =
{
	float2(0, 1),
	float2(0, 0),
	float2(1, 0),

	float2(1, 0),
	float2(1, 1),
	float2(0, 1)
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
    float3 Pos : POSITION;
};

struct GS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

//*********************************//
//			  Resources			   //
//*********************************//

Texture2D 		m_OldVelocityTex;
Texture2D 		m_NewPositionTex;

SamplerState 	m_SimpleSamplerPS;


//*********************************//
//			Vertex Shader		   //
//*********************************//
VS_OUTPUT VS( VS_INPUT input )
{
    VS_OUTPUT output;
    
    output.Pos = float3( 0,0,0 );
    
    return output;
}

//*********************************//
//			Geometry Shader		   //
//*********************************//
[ maxvertexcount( 6 ) ]
void GS( point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> SpriteStream )
{
    GS_OUTPUT output;
	
	// First triangle
	for( int i = 0; i < 3; i++ )
    {
        output.Pos = float4( g_positions[i], 1 );
        output.Tex = g_texcoords[i];
        
        SpriteStream.Append( output );
    }
	
    SpriteStream.RestartStrip();
	
	// Second triangle
	for( i = 3; i < 6; i++ )
    {
        output.Pos = float4( g_positions[i], 1 );
        output.Tex = g_texcoords[i];
        
        SpriteStream.Append( output );
    }
	
    SpriteStream.RestartStrip();
}

//*********************************//
//			Pixel Shader		   //
//*********************************//
float4 PS( PS_INPUT input ) : SV_Target
{
	// Impulsion (u/s)
	float aImpulsion = c_RenderTime * c_ForceImp * pow( 0.5f - sqrt( pow( input.Tex.x - ( c_PointImp.x / 512.0f ), 2 ) + pow( input.Tex.y - ( c_PointImp.y / 512.0f ), 2 ) ), 2 );

	// Old velocity texture
	float4 aVelocity = m_OldVelocityTex.Sample( m_SimpleSamplerPS, input.Tex );

	// Gravity (u/s)
	float aGravity = c_RenderTime * 0.98f * ( m_NewPositionTex.Sample( m_SimpleSamplerPS, input.Tex ).w / 20.0f );

	// Apply impulsion and gravity
	// If the particle are on the ground (y=0), we don't apply old velocity and gravity
	aVelocity.y = ( aVelocity.y - aGravity ) * step( -100.0f, m_NewPositionTex.Sample( m_SimpleSamplerPS, input.Tex ) ).y + aImpulsion;

	return aVelocity;

}