#include "Particule.h"


Particule::Particule( ID3D10Device *inDevice, IDXGISwapChain *inSwapChain )
{
	m_device = inDevice;

	m_SwapChain = inSwapChain;
	m_AdvanceShader = NULL;

	m_time = 0.0f;

	m_VelocityTextureTo		 = NULL ;
	m_VelocitySRVTo			 = NULL ;
	m_VelocityRTVTo			 = NULL ;
	m_VelocityTextureFrom	 = NULL ;
	m_VelocitySRVFrom		 = NULL ;
	m_VelocityRTVFrom		 = NULL ;
	m_PositionTextureTo		 = NULL ;
	m_PositionSRVTo			 = NULL ;
	m_PositionRTVTo			 = NULL ;
	m_PositionTextureFrom	 = NULL ;
	m_PositionSRVFrom		 = NULL ;
	m_PositionRTVFrom		 = NULL;
	m_ParticleSRV			 = NULL;
	m_ParticleTex			 = NULL;
	m_MainRTV				 = NULL ;
	m_VertexBuffer			 = NULL;
	m_BlendStateRP			 = NULL;
	m_BlendStateRTT			 = NULL;
	m_CstImpulsionFS		 = NULL ;
	m_CstRenderVS			 = NULL ;
	m_CstRenderGS			 = NULL ;
	
	m_ImpulsionShader = new HLShader( m_device ) ;
	m_ImpulsionShader->loadShaderFromFile( "../Shader/impulsion" ) ;

	m_AdvanceShader = new HLShader( m_device ) ;
	m_AdvanceShader->loadShaderFromFile( "../Shader/advance" ) ;

	m_RenderParticlesShader = new HLShader( m_device ) ;
	m_RenderParticlesShader->loadShaderFromFile( "../Shader/render" ) ;

	LARGE_INTEGER aFreq;

	QueryPerformanceFrequency( &aFreq );

	m_ClockFreq = ( double )aFreq.QuadPart / 1000.0;

	m_FrameRate  = 0.0;

	HRESULT aLastError = InitParticles();
	
	if( FAILED( aLastError ) )
	{
		ShowLastError( aLastError );
	}
}


Particule::~Particule()
{
	m_device = NULL;
	m_SwapChain = NULL;

	if( m_VelocityTextureTo		) m_VelocityTextureTo->Release();
	if( m_VelocitySRVTo			) m_VelocitySRVTo->Release();
	if( m_VelocityRTVTo			) m_VelocityRTVTo->Release();

	if( m_VelocityTextureFrom	) m_VelocityTextureFrom->Release();
	if( m_VelocitySRVFrom		) m_VelocitySRVFrom->Release();
	if( m_VelocityRTVFrom		) m_VelocityRTVFrom->Release();


	if( m_PositionTextureTo		) m_PositionTextureTo->Release();
	if( m_PositionSRVTo			) m_PositionSRVTo->Release();
	if( m_PositionRTVTo			) m_PositionRTVTo->Release();

	if( m_PositionTextureFrom	) m_PositionTextureFrom->Release();
	if( m_PositionSRVFrom		) m_PositionSRVFrom->Release();
	if( m_PositionRTVFrom		) m_PositionRTVFrom->Release();

	if( m_ParticleSRV			) m_ParticleSRV->Release( );
	if( m_ParticleTex			) m_ParticleTex->Release( );


	if( m_MainRTV				) m_MainRTV->Release();
	
	if( m_CstImpulsionFS		) m_CstImpulsionFS->Release();
	if( m_CstRenderVS			) m_CstRenderVS->Release();
	if( m_CstRenderGS			) m_CstRenderGS->Release();

	if( m_ViewEye				) delete m_ViewEye;

	if( m_VertexBuffer			) m_VertexBuffer->Release( );
	if( m_BlendStateRTT			) m_BlendStateRTT->Release( );
	if( m_BlendStateRP			) m_BlendStateRP->Release( );

	if( m_ImpulsionShader		) delete m_ImpulsionShader;
	if( m_AdvanceShader			) delete m_AdvanceShader;
	if( m_RenderParticlesShader ) delete m_RenderParticlesShader;
}

void Particule::ShowLastError( HRESULT inResult )
{
	LPTSTR aMsg;
	LPTSTR aTitle;
	DWORD aErr = GetLastError();

	// Get Message
	FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, inResult, 0,	( LPTSTR ) &aMsg, 0, NULL );
	FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, aErr, 0,	( LPTSTR ) &aTitle, 0, NULL );

	// Show Message
	MessageBox( NULL, aMsg, aTitle, MB_OK );
}


HRESULT Particule::InitParticles()
{
	// Error code
	HRESULT aErr = S_OK;

	// Create texture description, the same for the velocity/position texture (new and old)
	D3D10_TEXTURE2D_DESC aDsTex;
	aDsTex.Width = TEXTURE_SIDE;
	aDsTex.Height = TEXTURE_SIDE;
	aDsTex.MipLevels = 1;
	aDsTex.ArraySize = 1;
	aDsTex.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	aDsTex.Usage = D3D10_USAGE_DEFAULT;
	aDsTex.BindFlags = D3D10_BIND_SHADER_RESOURCE | D3D10_BIND_RENDER_TARGET;
	aDsTex.CPUAccessFlags = 0;
	aDsTex.MiscFlags = 0;
	aDsTex.SampleDesc.Count = 1;
	aDsTex.SampleDesc.Quality = 0;

	/* Fill initial data for position and velocity
	 Velocity :		XYZ -> Velocity of the particle
	 W	-> Weigh of the particle, used to compute gravity attraction
	 */
	D3DXVECTOR4* aDataPos = new D3DXVECTOR4[PARTICLE_MAX];
	D3DXVECTOR4* aDataVel = new D3DXVECTOR4[PARTICLE_MAX];

	int aIndex;

	for( int aCouche = 0; aCouche < 4; aCouche++ )
	{
		for( int aIndexZ = 0; aIndexZ < ( TEXTURE_SIDE / 4 ); aIndexZ++ )
		{
			for( int aIndexX = 0; aIndexX < ( TEXTURE_SIDE / 4 ); aIndexX++ )
			{
				aIndex = aIndexZ * ( TEXTURE_SIDE / 4 ) * aCouche + aIndexX;

				aDataPos[aIndex].x = ( float ) ( aIndexX - ( ( TEXTURE_SIDE / 4 ) / 2 ) ) * 3.0f;
				aDataPos[aIndex].y = -50.0f;
				aDataPos[aIndex].z = ( float ) ( aIndexZ - ( ( TEXTURE_SIDE / 4 ) / 2 ) ) / 1000.0f;
				aDataPos[aIndex].w = 1.0f;

				aDataVel[aIndex].x = 0.0f;
				aDataVel[aIndex].y = 0.0f;
				aDataVel[aIndex].z = 0.0f;

				if( aCouche == 0 )
				{
					aDataVel[aIndex].w = 1.0f;
				}

				else
				{
					// Random between 0.01 and 1.0
					aDataVel[aIndex].w = ( float ) ( rand( ) % 100 + 1 ) / 100.0f * 20.0f;
				}
			}
		}
	}

	// Create texture from data
	// Set data to fill in textures
	D3D10_SUBRESOURCE_DATA InitData[2];

	// Pos
    InitData[0].pSysMem = aDataPos;
    InitData[0].SysMemPitch = sizeof( D3DXVECTOR4 ) * TEXTURE_SIDE;
    InitData[0].SysMemSlicePitch = sizeof( D3DXVECTOR4 ) * PARTICLE_MAX;

	// Vel 
    InitData[1].pSysMem = aDataVel;
    InitData[1].SysMemPitch = sizeof( D3DXVECTOR4 ) * TEXTURE_SIDE;
    InitData[1].SysMemSlicePitch = sizeof( D3DXVECTOR4 ) * PARTICLE_MAX;
	
	if( FAILED( aErr = m_device->CreateTexture2D( &aDsTex, &InitData[0], &m_PositionTextureFrom ) ) )
	{
		return aErr;
	}
	if( FAILED( aErr = m_device->CreateTexture2D( &aDsTex, &InitData[0], &m_PositionTextureTo ) ) )
	{
		return aErr;
	}
	
	if( FAILED( aErr = m_device->CreateTexture2D( &aDsTex, &InitData[1], &m_VelocityTextureFrom ) ) )
	{
		return aErr;
	}
	if( FAILED( aErr = m_device->CreateTexture2D( &aDsTex, &InitData[1], &m_VelocityTextureTo ) ) )
	{
		return aErr;
	}
	
	// dump memory
    if( aDataPos ) delete aDataPos;
    if( aDataVel ) delete aDataVel;

	// Create the Shader Resource views
	D3D10_SHADER_RESOURCE_VIEW_DESC aSRVDesc;

    ZeroMemory( &aSRVDesc, sizeof( aSRVDesc ) );

    aSRVDesc.Format								= aDsTex.Format;
    aSRVDesc.ViewDimension						= D3D10_SRV_DIMENSION_TEXTURE2DARRAY;
    aSRVDesc.Texture2DArray.ArraySize			= 1;
    aSRVDesc.Texture2DArray.FirstArraySlice		= 0;
    aSRVDesc.Texture2DArray.MipLevels			= 1;
    aSRVDesc.Texture2DArray.MostDetailedMip		= 0;
	
	if( FAILED( aErr = m_device->CreateShaderResourceView( m_PositionTextureFrom, &aSRVDesc, &m_PositionSRVFrom ) ) )
	{
		return aErr;
	}
	if( FAILED( aErr = m_device->CreateShaderResourceView( m_PositionTextureTo, &aSRVDesc, &m_PositionSRVTo ) ) )
	{
		return aErr;
	}
	
	if( FAILED( aErr = m_device->CreateShaderResourceView( m_VelocityTextureFrom, &aSRVDesc, &m_VelocitySRVFrom ) ) )
	{
		return aErr;
	}
	if( FAILED( aErr = m_device->CreateShaderResourceView( m_VelocityTextureTo, &aSRVDesc, &m_VelocitySRVTo ) ) )
	{
		return aErr;
	}


	// Load texture from file
	if( FAILED( aErr = D3DX10CreateTextureFromFileW( m_device, L"../Resources/particle.png", NULL, NULL, &m_ParticleTex, NULL ) ) )
	{
		return aErr;
	}

	// Create resource view
	D3D10_SHADER_RESOURCE_VIEW_DESC aRvDesc;
	aRvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	aRvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	aRvDesc.Texture2D.MostDetailedMip = 0;
	aRvDesc.Texture2D.MipLevels = -1;

	if( FAILED( aErr = m_device->CreateShaderResourceView( m_ParticleTex, &aRvDesc, &m_ParticleSRV ) ) )
	{
		return aErr;
	}
	

	// Create the Render Target Views
	D3D10_RENDER_TARGET_VIEW_DESC aRTVDesc;

    ZeroMemory( &aRTVDesc, sizeof( aRTVDesc ) );

    aRTVDesc.Format								= aDsTex.Format;
    aRTVDesc.ViewDimension						= D3D10_RTV_DIMENSION_TEXTURE2DARRAY;
    aRTVDesc.Texture2DArray.ArraySize			= 1;
    aRTVDesc.Texture2DArray.FirstArraySlice		= 0;
    aRTVDesc.Texture2DArray.MipSlice			= 0;
	
	if( FAILED( aErr = m_device->CreateRenderTargetView( m_PositionTextureFrom, &aRTVDesc, &m_PositionRTVFrom ) ) )
	{
		return aErr;
	}
	if( FAILED( aErr = m_device->CreateRenderTargetView( m_PositionTextureTo, &aRTVDesc, &m_PositionRTVTo ) ) )
	{
		return aErr;
	}

	if( FAILED( aErr = m_device->CreateRenderTargetView( m_VelocityTextureFrom, &aRTVDesc, &m_VelocityRTVFrom ) ) )
	{
		return aErr;
	}
	if( FAILED( aErr = m_device->CreateRenderTargetView( m_VelocityTextureTo, &aRTVDesc, &m_VelocityRTVTo ) ) )
	{
		return aErr;
	}

	// BackBuffer target
	ID3D10Texture2D* aBuffer;
    if ( FAILED ( aErr = m_SwapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), ( LPVOID* )&aBuffer) ) )
    {
        return aErr;
	}
	
    if( FAILED( aErr = m_device->CreateRenderTargetView( aBuffer, NULL, &m_MainRTV ) ) )
	{
		return aErr;
	}

	if ( aBuffer ) aBuffer->Release();

	// Create vertex buffer
	 D3D10_BUFFER_DESC aVbDesc =
    {
		PARTICLE_MAX * sizeof( PARTICLE_VERTEX ),
        D3D10_USAGE_DEFAULT,
        D3D10_BIND_VERTEX_BUFFER,
        0,
        0
    };
	 
	D3D10_SUBRESOURCE_DATA aVbInitData;
    ZeroMemory( &aVbInitData, sizeof( D3D10_SUBRESOURCE_DATA ) );

    PARTICLE_VERTEX* aVertices = new PARTICLE_VERTEX[ PARTICLE_MAX ];

    if( !aVertices )
        return E_OUTOFMEMORY;


    for( UINT aIndex = 0; aIndex < PARTICLE_MAX; aIndex++ )
    {
        aVertices[aIndex].Pos = D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 1.0f );
    }

    aVbInitData.pSysMem = aVertices;
    if( FAILED( aErr = m_device->CreateBuffer( &aVbDesc, &aVbInitData, &m_VertexBuffer ) ) )
	{
		return aErr;
	}

    if( aVertices ) delete aVertices;

	// Create and fill constant buffers for rendering
	{
		// Set Projection
		D3DXMatrixPerspectiveFovLH( &m_MatrixProj, 0.78f, 1.0f, 0.1f, 10000.0f );

		// Set view position
		D3DXVECTOR3 aViewUp( 0.0f, 1.0f, 0.0f ),
					aViewAt( 0.0f, 0.0f, 0.0f );
		m_ViewEye = new D3DXVECTOR3( 0.0f, 0.0f, -6000.0f );

		D3DXMatrixLookAtLH( &m_MatrixView, m_ViewEye, &aViewAt, &aViewUp );

		// Set particle emiter position
		D3DXMatrixIdentity( &m_MatrixEmiter );

		// Fill buffer structure
		CB_RENDER_VS aConstRenderVS;
		aConstRenderVS.TexSideSize	= TEXTURE_SIDE;

		CB_RENDER_GS aConstRenderGS;
		aConstRenderGS.Proj = m_MatrixProj;
		aConstRenderGS.View = m_MatrixView;

		//Set buffer desc
		D3D10_BUFFER_DESC aCbDesc[2];
		aCbDesc[0].ByteWidth		= sizeof( CB_RENDER_VS );
		aCbDesc[0].Usage			= D3D10_USAGE_DEFAULT;
		aCbDesc[0].BindFlags		= D3D10_BIND_CONSTANT_BUFFER;
		aCbDesc[0].CPUAccessFlags	= 0;
		aCbDesc[0].MiscFlags		= 0;
		aCbDesc[1].ByteWidth		= sizeof( CB_RENDER_GS );
		aCbDesc[1].Usage			= D3D10_USAGE_DYNAMIC;
		aCbDesc[1].BindFlags		= D3D10_BIND_CONSTANT_BUFFER;
		aCbDesc[1].CPUAccessFlags	= D3D10_CPU_ACCESS_WRITE;
		aCbDesc[1].MiscFlags		= 0;

		// Fill in subresource data
		D3D10_SUBRESOURCE_DATA aInitData[2];
		aInitData[0].pSysMem			= &aConstRenderVS;
		aInitData[0].SysMemPitch		= 0;
		aInitData[0].SysMemSlicePitch	= 0;
		aInitData[1].pSysMem			= &aConstRenderGS;
		aInitData[1].SysMemPitch		= 0;
		aInitData[1].SysMemSlicePitch	= 0;
		
		// Create the buffer
		if( FAILED( aErr = m_device->CreateBuffer( &aCbDesc[0], &aInitData[0], &m_CstRenderVS ) ) )
		{
			return aErr;
		}

		// Create the buffer
		if( FAILED( aErr = m_device->CreateBuffer( &aCbDesc[1], &aInitData[1], &m_CstRenderGS ) ) )
		{
			return aErr;
		}
	}

	// Create and fill constant buffers for impulsion shader
	{
		// Fill buffer structure
		CB_IMPULSION_FS aConstImp;
		aConstImp.RenderTime = 0.0001f;
		aConstImp.ImpulsionForce = 0;
		aConstImp.ImpulsionPoint = D3DXVECTOR2( 0.0f, 0.0f );

		//Set buffer desc
		D3D10_BUFFER_DESC aCbDesc;
		aCbDesc.ByteWidth		= sizeof( CB_IMPULSION_FS );
		aCbDesc.Usage			= D3D10_USAGE_DYNAMIC;
		aCbDesc.BindFlags		= D3D10_BIND_CONSTANT_BUFFER;
		aCbDesc.CPUAccessFlags	= D3D10_CPU_ACCESS_WRITE;
		aCbDesc.MiscFlags		= 0;

		// Fill in subresource data
		D3D10_SUBRESOURCE_DATA aInitData;
		aInitData.pSysMem			= &aConstImp;
		aInitData.SysMemPitch		= 0;
		aInitData.SysMemSlicePitch	= 0;

		// Create the buffer
		if( FAILED( aErr = m_device->CreateBuffer( &aCbDesc, &aInitData, &m_CstImpulsionFS ) ) )
		{
			return aErr;
		}
	}

	// Create blend state
	D3D10_BLEND_DESC aBlendStateRTT;
	ZeroMemory( &aBlendStateRTT, sizeof( D3D10_BLEND_DESC ) );

	aBlendStateRTT.BlendEnable[0] = FALSE;
	aBlendStateRTT.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;

	m_device->CreateBlendState( &aBlendStateRTT, &m_BlendStateRTT );

	D3D10_BLEND_DESC aBlendStateRP;
	ZeroMemory( &aBlendStateRP, sizeof( D3D10_BLEND_DESC ) );

	aBlendStateRP.BlendEnable[0]			= TRUE;
	aBlendStateRP.SrcBlend					= D3D10_BLEND_SRC_ALPHA;
	aBlendStateRP.DestBlend					= D3D10_BLEND_ONE;
	aBlendStateRP.BlendOp					= D3D10_BLEND_OP_ADD;
	aBlendStateRP.SrcBlendAlpha				= D3D10_BLEND_ZERO;
	aBlendStateRP.DestBlendAlpha			= D3D10_BLEND_ZERO;
	aBlendStateRP.BlendOpAlpha				= D3D10_BLEND_OP_ADD;
	aBlendStateRP.RenderTargetWriteMask[0]	= D3D10_COLOR_WRITE_ENABLE_ALL;

	m_device->CreateBlendState( &aBlendStateRP, &m_BlendStateRP );

	return aErr;
}

	
void Particule::SwapPosTextures()
{
	ID3D10Texture2D				*Temp1 = m_PositionTextureTo;
    ID3D10ShaderResourceView	*Temp2 = m_PositionSRVTo;
    ID3D10RenderTargetView		*Temp3 = m_PositionRTVTo;

    m_PositionTextureTo		=	m_PositionTextureFrom;
    m_PositionSRVTo			=	m_PositionSRVFrom;
    m_PositionRTVTo			=	m_PositionRTVFrom;

    m_PositionTextureFrom	=	Temp1;
    m_PositionSRVFrom		=	Temp2;
    m_PositionRTVFrom		=	Temp3;
}


void Particule::SwapVelTextures()
{
	ID3D10Texture2D				*Temp1 = m_VelocityTextureTo;
    ID3D10ShaderResourceView	*Temp2 = m_VelocitySRVTo;
    ID3D10RenderTargetView		*Temp3 = m_VelocityRTVTo;

    m_VelocityTextureTo		=	m_VelocityTextureFrom;
    m_VelocitySRVTo			=	m_VelocitySRVFrom;
    m_VelocityRTVTo			=	m_VelocityRTVFrom;

    m_VelocityTextureFrom	=	Temp1;
    m_VelocitySRVFrom		=	Temp2;
    m_VelocityRTVFrom		=	Temp3;
}

void Particule::ComputeImpulsion()
{

	// Compute frequencies from spectrum

    // Store the old viewport
    UINT aNumVP = 1;
    D3D10_VIEWPORT aOldVP;
    m_device->RSGetViewports( &aNumVP, &aOldVP );

    // Set the new render targets
    m_device->OMSetRenderTargets( 1, &m_VelocityRTVTo, NULL );
	m_device->OMSetBlendState( m_BlendStateRTT, 0, 0xffffffff );

	float aClearColor[4] = { 0.125f, 0.3f, 0.125f, 1.0f };
	m_device->ClearRenderTargetView( m_VelocityRTVTo, aClearColor );

    // Set the new viewport
	D3D10_VIEWPORT aNewVP;

    aNewVP.Width		= TEXTURE_SIDE;
    aNewVP.Height		= TEXTURE_SIDE;
    aNewVP.TopLeftX		= 0;
    aNewVP.TopLeftY		= 0;
    aNewVP.MinDepth		= 0.0f;
    aNewVP.MaxDepth		= 1.0f;

    m_device->RSSetViewports( 1, &aNewVP );

	// Use Impulsion shader + Primitive topology
	m_ImpulsionShader->bind();

    m_device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_POINTLIST );

	// Set constants
	m_device->PSSetConstantBuffers( 0, 1, &m_CstImpulsionFS );
	
	{
		CB_IMPULSION_FS *aConstSpectrum = NULL;

		m_CstImpulsionFS->Map( D3D10_MAP_WRITE_DISCARD, 0, ( void** ) &aConstSpectrum );

		if( m_time > 2000 )
		{
			aConstSpectrum->ImpulsionPoint = D3DXVECTOR2( 0.0f, 0.0f );
			aConstSpectrum->ImpulsionForce = 1000.0f;
			
			m_time = 0.0f;
		}

		else
		{
			aConstSpectrum->ImpulsionForce = 0.0f;
		}

		aConstSpectrum->RenderTime = ( float ) m_FrameRate / 1000.0f;

		m_CstImpulsionFS->Unmap( );
	}

	// Set texture source
	ID3D10ShaderResourceView* aSRV[2] = { m_VelocitySRVFrom, m_PositionSRVFrom };
	m_device->PSSetShaderResources( 0, 2, aSRV );

	// Draw
	m_device->Draw( 1, 0 );

	// Unset texture source
    ID3D10ShaderResourceView* aNULLs[2] = { 0, 0 };
    m_device->PSSetShaderResources( 0, 2, aNULLs );

	// Unbind shader
	m_ImpulsionShader->unbind();

	// Restore old viewport
    m_device->RSSetViewports( 1, &aOldVP );
}


void Particule::AdvanceParticles()
{
    // Store the old viewport
    UINT aNumVP = 1;
    D3D10_VIEWPORT aOldVP;
    m_device->RSGetViewports( &aNumVP, &aOldVP );

    // Set the new render targets
    m_device->OMSetRenderTargets( 1, &m_PositionRTVTo, NULL );
	m_device->OMSetBlendState( m_BlendStateRTT, 0, 0xffffffff );

    // Set the new viewport
	D3D10_VIEWPORT aNewVP;

    aNewVP.Width		= TEXTURE_SIDE;
    aNewVP.Height		= TEXTURE_SIDE;
    aNewVP.TopLeftX		= 0;
    aNewVP.TopLeftY		= 0;
    aNewVP.MinDepth		= 0.0f;
    aNewVP.MaxDepth		= 1.0f;

    m_device->RSSetViewports( 1, &aNewVP );

	// Use Impulsion shader + Primitive topology
	m_AdvanceShader->bind();

    m_device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_POINTLIST );
		

	// Set texture source
	ID3D10ShaderResourceView* aSRV[2] = { m_PositionSRVFrom , m_VelocitySRVTo };
	m_device->PSSetShaderResources( 0, 2, aSRV );

	// Draw
	m_device->Draw( 1, 0 );

	// Unset texture source
    ID3D10ShaderResourceView* aNULLs[2] = { 0, 0};
    m_device->PSSetShaderResources( 0, 2, aNULLs );

	// Unbind shader
	m_AdvanceShader->unbind();

	// Restore old viewport
    m_device->RSSetViewports( 1, &aOldVP );
}

void Particule::RenderParticles()
{
    // Set the new render targets
	m_device->OMSetRenderTargets( 1, &m_MainRTV, NULL );
	m_device->OMSetBlendState( m_BlendStateRP, 0, 0xffffffff );

	// Use Impulsion shader + Primitive topology
	m_RenderParticlesShader->bind();
	
	ID3D10Buffer* aBuffers[1] = { m_VertexBuffer };
    UINT aStride[1] = { sizeof( PARTICLE_VERTEX ) };
    UINT aOffset[1] = { 0 };
    m_device->IASetVertexBuffers( 0, 1, aBuffers, aStride, aOffset );

    m_device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_POINTLIST );
	
	// Set constants
	m_device->VSSetConstantBuffers( 1, 1, &m_CstRenderVS ) ;
	m_device->GSSetConstantBuffers( 2, 1, &m_CstRenderGS ) ;

	{
		CB_RENDER_GS *aConstRenderGS;

		m_CstRenderGS->Map( D3D10_MAP_WRITE_DISCARD, 0, ( void** ) &aConstRenderGS );

		D3DXVECTOR3 aViewUp( 0.0f, 1.0f, 0.0f ),
			aViewAt(0.0f, 0.0f, 0.0f );
		D3DXMatrixLookAtLH( &m_MatrixView, m_ViewEye, &aViewAt, &aViewUp );

		aConstRenderGS->View = m_MatrixView;
		aConstRenderGS->Proj = m_MatrixProj;

		m_CstRenderGS->Unmap( );
	}

	// Set texture source
	ID3D10ShaderResourceView* aSRVVS[1] = { m_PositionSRVTo };
	ID3D10ShaderResourceView* aSRVPS[1] = { m_ParticleSRV };
	m_device->VSSetShaderResources( 0, 1, aSRVVS );
	m_device->PSSetShaderResources( 1, 1, aSRVPS );

	// Draw
	m_device->Draw( PARTICLE_MAX, 0);

	// Unset texture source
	ID3D10ShaderResourceView* aNULLVS[1] = { 0 };
	ID3D10ShaderResourceView* aNULLPS[1] = { 0 };
	m_device->VSSetShaderResources( 0, 1, aNULLVS );
	m_device->PSSetShaderResources( 1, 1, aNULLPS );

	// Unbind shader
	m_RenderParticlesShader->unbind();

	m_SwapChain->Present( 0, 0 );

}

void Particule::render()
{
    float aClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_device->ClearRenderTargetView( m_MainRTV, aClearColor );
	
	// Start timer
	QueryPerformanceCounter( &m_StartCounter );

	//Render
	ComputeImpulsion();
	AdvanceParticles();
	RenderParticles();
	
	SwapPosTextures();
	SwapVelTextures();

	// End timer and print framerate
	QueryPerformanceCounter( &m_EndCounter );
	m_FrameRate = double( m_EndCounter.QuadPart - m_StartCounter.QuadPart) / m_ClockFreq;
	m_time += ( float ) m_FrameRate;
}

double Particule::getFrameRate()
{
	return m_FrameRate;
}

D3DXVECTOR3 Particule::getCameraPos()
{
	return *m_ViewEye;
}

void Particule::IncY( )
{
	m_ViewEye->y += 10.0f;
}
void Particule::DecY( )
{
	m_ViewEye->y -= 10.0f;
}

void Particule::IncX( )
{
	m_ViewEye->x += 10.0f;
}
void Particule::DecX( )
{
	m_ViewEye->x -= 10.0f;
}

void Particule::IncZ( )
{
	m_ViewEye->z += 10.0f;
}
void Particule::DecZ( )
{
	m_ViewEye->z -= 10.0f;
}