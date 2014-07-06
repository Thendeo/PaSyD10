#ifndef HLSHADER_H
#define HLSHADER_H

#include <Windows.h>

#include <D3D10.h>
#include <D3DX10math.h>

#include <vector>
#include <string>
#include <fstream>

#include <assert.h>

typedef std::vector<unsigned char> BytesVector;

struct PARTICLE_VERTEX
{
    D3DXVECTOR4 Pos;
};

class HLShader
{

private:

	//! Reference to the current device used
	ID3D10Device			*m_device;

	//! Vertex Shader
	ID3D10VertexShader		*m_VertexShader;

	//! Vertex constant buffer
	ID3D10Buffer			*m_VertexCstBuffer;
	
	//! Geometry Shader
	ID3D10GeometryShader	*m_GeometryShader;

	//! Geometry constant buffer
	ID3D10Buffer			*m_GeometryCstBuffer;

	//! Pixel Shader
	ID3D10PixelShader		*m_PixelShader;

	//! Pixel constant buffer
	ID3D10Buffer			*m_PixelCstBuffer;

	//! Sampler state of the shader
	ID3D10SamplerState		*m_SamplerState;

	//! Input Layout
	ID3D10InputLayout		*m_InputLayout;

	/*! Read source from binary file
	 * /param[out] outContent BytesVector to fill
	 * /param[in]  inFilePath Path to the file we want to load
	 */
	void readBinaryFile( BytesVector &outContent, const std::string &filepath);

public:

	/*! Constructor
	 * /param[in] inDevice Current device used
	 */
	HLShader( ID3D10Device* inDevice );

	//! Desctructor
	~HLShader();

	/*! Load shader from three files
	 * /param[in]  inFilename Name of the shader source we want to load
	 */
	HRESULT loadShaderFromFile( const std::string inFileName );
	
	//! Use this shader
	void bind();
	
	//! Unuse this shader
	void unbind();

	//! Unuse GS shader
	void unbindGS();

	//! Get reference to the vertex cst buffer
	ID3D10Buffer* getVertexCstBuffer();

	//! Get reference to the vertex cst buffer
	ID3D10Buffer* getGeometryCstBuffer();

	//! Get reference to the vertex cst buffer
	ID3D10Buffer* getPixelCstBuffer();



};

#endif //HLSHADER_H

