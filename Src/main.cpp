/*! main.cpp
 *  Alexis LAFONT
 *  2014
 */

#include "Device.h"

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	Device *aDevice = new Device( hInstance, nCmdShow );

	// Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            aDevice->Render();
        }
    }

   delete aDevice;

   return ( int )msg.wParam;
}