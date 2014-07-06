#ifndef PARTICULE_H
#define PARTICULE_H

#include "HLShader.h"

#define TEXTURE_SIDE 1024
#define PARTICLE_MAX ( ( TEXTURE_SIDE * TEXTURE_SIDE ) + 1 )

__declspec(align(16))
struct CB_RENDER_VS
{
	float 		TexSideSize;
};

__declspec(align(16))
struct CB_RENDER_GS
{
	D3DXMATRIX 	View;
	D3DXMATRIX 	Proj;
};

__declspec(align(16))
struct CB_IMPULSION_FS
{
	D3DXVECTOR2	ImpulsionPoint;
	float		ImpulsionForce;
	float		RenderTime;
};

class Particule
{
private: 
	
	//! Reference to the directx device
	ID3D10Device			*m_device;
	
	//! Reference to the current SwapChain
	IDXGISwapChain			*m_SwapChain;

	/********************
		    Veloctiy
	********************/

	//! Texture target
	ID3D10Texture2D						*m_VelocityTextureTo;
	//! Shader Resource view that point to the texture target
	ID3D10ShaderResourceView			*m_VelocitySRVTo;
	//! RTV to render texture target
	ID3D10RenderTargetView				*m_VelocityRTVTo;

	//! Texture input
	ID3D10Texture2D						*m_VelocityTextureFrom;
	//! Shader Resource view that point to the texture input
	ID3D10ShaderResourceView			*m_VelocitySRVFrom;
	//! RTV to render texture input
	ID3D10RenderTargetView				*m_VelocityRTVFrom;


	/********************
		   Position
	********************/

	//! Texture target
	ID3D10Texture2D						*m_PositionTextureTo;
	//! Shader Resource view that point to the texture target
	ID3D10ShaderResourceView			*m_PositionSRVTo;
	//! RTV to render texture target
	ID3D10RenderTargetView				*m_PositionRTVTo;

	//! Texture input
	ID3D10Texture2D						*m_PositionTextureFrom;
	//! Shader Resource view that point to the texture input
	ID3D10ShaderResourceView			*m_PositionSRVFrom;
	//! RTV to render texture input
	ID3D10RenderTargetView				*m_PositionRTVFrom;
	
	/********************
		Particle 
	********************/

	//! Particle texture for billboard rendering
	ID3D10Resource						*m_ParticleTex;

	//! Particle srv for billboard rendering
	ID3D10ShaderResourceView			*m_ParticleSRV;

	/********************
		   Render
	********************/

	//! Render target view binded to back buffer
	ID3D10RenderTargetView				*m_MainRTV;
	

	/********************
		   Shaders
	********************/

	//! Shader to update impulsion and velocity
	HLShader							*m_ImpulsionShader;

	//! Shader to advance particles
	HLShader							*m_AdvanceShader;

	//! Shader to render particles
	HLShader							*m_RenderParticlesShader;

		
	/********************
		   Buffers
	********************/

	/*! Vertex Buffer used to render all the particle
	 *  Contain PARTICLE_MAX points
	 */
	ID3D10Buffer						*m_VertexBuffer;
	
	//! Constant buffer sent to the pixel stage of the impulsion shader
	ID3D10Buffer						*m_CstImpulsionFS;
	
	//! Constant buffer sent to the vertex stage of the render shader
	ID3D10Buffer						*m_CstRenderVS;

	//! Constant buffer sent to the geometry stage of therender shader
	ID3D10Buffer						*m_CstRenderGS;

	//! Camera position
	D3DXVECTOR3							*m_ViewEye;

	
	/********************
		   Matrix
	********************/
	
	//! Matrix representing the eye of the camera
	D3DXMATRIX							m_MatrixView;
	
	//! Matrix representing the projection of the camera
	D3DXMATRIX							m_MatrixProj;

	//! Matrix representing the center of the particle emitter
	D3DXMATRIX							m_MatrixEmiter;


	//! Blend state for render to texture
	ID3D10BlendState					*m_BlendStateRTT;

	//! Blend state used to render particle with alpha
	ID3D10BlendState					*m_BlendStateRP;


	/********************
			Time
	********************/

	//! Clock frequency const
	double					m_ClockFreq;

	//! Time when render start
	LARGE_INTEGER			m_StartCounter;

	//! Time when render end
	LARGE_INTEGER			m_EndCounter;

	//! Time between two render
	double					m_FrameRate;

	//! Current time
	float					m_time;
	
	/********************
		   Functions
	********************/

	//! Print the last error in a message box style
	void ShowLastError( HRESULT inResult );

	//! Init particle effects and resources
	HRESULT InitParticles();
	
	//! Swap "from" and "to" position texture, the render texture became the input of the pixel shader, to render the new position texture
	void SwapPosTextures();

	//! Swap "from" and "to" velocity texture, the render texture became the input of the pixel shader, to render the new velocity texture
	void SwapVelTextures();
	
	//! Compute the impulsion from the song, update velocity texture
	void ComputeImpulsion();

	//! Update the position of the particles
	void AdvanceParticles();

	//! Render particle to back buffer
	void RenderParticles();

public:
	
	/*! Constructor
	 * /param[in] inDevice Reference to the current device used
	 * /param[in] inSwapChain Reference to the current SwapChain used
	 */
	Particule( ID3D10Device *inDevice, IDXGISwapChain *inSwapChain );

	//! Destructor
	~Particule();

	//! Render Particles
	void render();

	//! Get current frame rate
	double getFrameRate( );

	//! Get current camera pos
	D3DXVECTOR3 getCameraPos( );

	//! Inc Y position
	void IncY( );
	//! Dec Y position
	void DecY( );

	//! Inc X position
	void IncX( );
	//! Dec X position
	void DecX( );

	//! Inc Z position
	void IncZ( );
	//! Dec Z position
	void DecZ( );
};

#endif //PARTICULE_H
