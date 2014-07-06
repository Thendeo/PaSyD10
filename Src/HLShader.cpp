#include "HLShader.h"


HLShader::HLShader( ID3D10Device* inDevice )
{
	m_device = inDevice;

	m_VertexShader			= NULL;
	m_GeometryShader		= NULL;
	m_PixelShader			= NULL;

	m_VertexCstBuffer	= NULL;
	m_GeometryCstBuffer	= NULL;
	m_PixelCstBuffer	= NULL;
}


HLShader::~HLShader()
{
	m_device = NULL;

	if( m_VertexShader )		m_VertexShader->Release();
	if( m_GeometryShader )		m_GeometryShader->Release();
	if( m_PixelShader )			m_PixelShader->Release();
	
	if( m_VertexCstBuffer )		m_VertexCstBuffer->Release();
	if( m_GeometryCstBuffer )	m_GeometryCstBuffer->Release();
	if( m_PixelCstBuffer )		m_PixelCstBuffer->Release();

	if( m_SamplerState )		m_SamplerState->Release();
	if( m_InputLayout )			m_InputLayout->Release();
}

void HLShader::readBinaryFile( BytesVector &outContent, const std::string &inFilePath)
{
	std::ifstream is(inFilePath.c_str(),std::ios_base::binary|std::ios_base::in);
	is.seekg((std::streamoff)0,std::ios_base::end);
	assert(is);
	outContent.resize((size_t)is.tellg());
	is.seekg((std::streamoff)0,std::ios_base::beg);
	is.read((char*)&outContent[0],outContent.size());
}

HRESULT HLShader::loadShaderFromFile( const std::string inFileName )
{
	HRESULT aErr = S_OK;
	BytesVector aVsData, aGsData, aFsData;
	
	// Load VS
	readBinaryFile( aVsData, inFileName+".vso" );
	if( FAILED( aErr = m_device->CreateVertexShader( &aVsData[0], aVsData.size(), &m_VertexShader ) ) )
	{
		return aErr;
	}

	// Load GS
	readBinaryFile( aGsData, inFileName+".gso"  );
	if( FAILED( aErr = m_device->CreateGeometryShader( &aGsData[0], aGsData.size(), &m_GeometryShader ) ) )
	{
		return aErr;
	}
	
	// Load PS
	readBinaryFile( aFsData, inFileName+".fso"  );
	if( FAILED( aErr = m_device->CreatePixelShader( &aFsData[0], aFsData.size(), &m_PixelShader ) ) )
	{
		return aErr;
	}

	// Create sampler state
	D3D10_SAMPLER_DESC aSamplerDesc = 
	{
		D3D10_FILTER_MIN_MAG_MIP_LINEAR,
		D3D10_TEXTURE_ADDRESS_CLAMP,
		D3D10_TEXTURE_ADDRESS_CLAMP,
		D3D10_TEXTURE_ADDRESS_CLAMP,
		0.0f,
		1,
		D3D10_COMPARISON_NEVER,
		{0.0f,0.0f,0.0f,0.0f},
		0.0f,
		D3D10_FLOAT32_MAX
	};

	if ( FAILED( aErr = m_device->CreateSamplerState( &aSamplerDesc, &m_SamplerState ) ) )
	{
		return aErr;
	}

	
	// Create Input Layout (only pos and tex required by all shaders for now)
	D3D10_INPUT_ELEMENT_DESC aLayout[] =
	{
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};

	if( FAILED( aErr = m_device->CreateInputLayout( aLayout, sizeof( aLayout ) / sizeof( D3D10_INPUT_ELEMENT_DESC ), &aVsData[0], aVsData.size(), &m_InputLayout ) ) )
	{
		return aErr;
	}

	return aErr;
}

	
void HLShader::bind()
{
	m_device->IASetInputLayout( m_InputLayout );
	
	m_device->PSSetSamplers( 0, 1, &m_SamplerState );

	m_device->VSSetShader( m_VertexShader );
	m_device->GSSetShader( m_GeometryShader );
	m_device->PSSetShader( m_PixelShader );

}

void HLShader::unbind()
{
	m_device->VSSetShader( NULL ) ;
	m_device->GSSetShader( NULL ) ;
	m_device->PSSetShader( NULL ) ;
}

void HLShader::unbindGS()
{
	m_device->GSSetShader( NULL ) ;
}


ID3D10Buffer* HLShader::getVertexCstBuffer()
{
	return m_VertexCstBuffer;
}

ID3D10Buffer* HLShader::getGeometryCstBuffer()
{
	return m_GeometryCstBuffer;
}

ID3D10Buffer* HLShader::getPixelCstBuffer()
{
	return m_PixelCstBuffer;
}
