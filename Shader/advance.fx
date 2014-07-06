//--------------------------------------------------------------------------------------
// File: advance.fx
//
// Alexis LAFONT - 2014
//--------------------------------------------------------------------------------------

//*********************************//
//			Constant buffer		   //
//*********************************//

static const float3 g_positions[6] =
{
	float3(-1, -1, 0),
	float3(-1,  1, 0),
	float3( 1,  1, 0),

	float3( 1,  1, 0),
	float3( 1, -1, 0),
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

Texture2D 		m_OldPositionTex ;
Texture2D 		m_NewVelocityTex ;

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
	
	float3 aPosition = m_OldPositionTex.Sample( m_SimpleSamplerPS, input.Tex.xy ).xyz + m_NewVelocityTex.Sample( m_SimpleSamplerPS, input.Tex.xy ).xyz;

	return float4( aPosition, m_NewVelocityTex.Sample( m_SimpleSamplerPS, input.Tex.xy ).w );
	
}