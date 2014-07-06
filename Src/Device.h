#ifndef DEVICE_H
#define DEVICE_H

#include "Particule.h"

class Device
{

private:
	
	/****************************
		 Private members		
	*****************************/

	//! Handle of the window
	HWND m_hWnd;

	//! Handle of the device context
	HDC m_hDC;

	//! Viewport of the window
	RECT m_clientRect;

	//! Text buffer used to raw frame rate
	char m_TextBuffer[200];

	//! DirectX device
	ID3D10Device		*m_d3dDevice;

	//! SwapChain
	IDXGISwapChain		*m_SwapChain;
	
	//! Height and width of the window
	UINT m_width, m_height;

	//! Particle class, used to render particle system
	Particule *m_Particule;
	
	/****************************
		 Private functions		
	*****************************/

	//! Print the last error in a message box style
	void ShowLastError( HRESULT inResult );

	/*! Initialisation of the windows' program
	 * /param[in] hInstance Instance of the windows
	 * /param[in] nCmdShow Parameter for the window
	 */
	HRESULT InitWindow ( HINSTANCE hInstance, int nCmdShow );

	/*! Initialisation of the DirectX10 device
	 *
	 */
	HRESULT InitDevice();

	/*! Message management function give to he window handle
	*
	*/
	static LRESULT CALLBACK WProc( HWND, UINT, WPARAM, LPARAM );

	/*! Real window proc message callback called from the statc function
	*
	*/
	LRESULT CALLBACK RealWProc( HWND, UINT, WPARAM, LPARAM );
	
public:

	/*! Constructor
	 * /param[in] hInstance Instance of the windows
	 * /param[in] nCmdShow Parameter for the window
	 */
	Device( HINSTANCE hInstance, int nCmdShow  );

	//! Destructor
	~Device();

	//! Main render function
	void Render();
};

#endif //DEVICE_H