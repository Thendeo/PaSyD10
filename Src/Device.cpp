#include "Device.h"


Device::Device( HINSTANCE hInstance, int nCmdShow )
{

	m_hWnd = NULL;
	m_d3dDevice = NULL;
	m_SwapChain = NULL;
	m_width = NULL;
	m_height = NULL;
	m_Particule = NULL;

	HRESULT aLastError;
	aLastError = InitWindow( hInstance, nCmdShow );

	if( FAILED( aLastError ) )
	{
		ShowLastError( aLastError );
	}

	aLastError = InitDevice( );

	if( FAILED( aLastError ) )
	{
		ShowLastError( aLastError );
	}

	m_Particule = new Particule( m_d3dDevice, m_SwapChain );

}


Device::~Device( )
{
	if( m_d3dDevice ) m_d3dDevice->ClearState( );
	if( m_Particule ) delete m_Particule;

	if( m_SwapChain ) m_SwapChain->Release( );
	if( m_d3dDevice ) m_d3dDevice->Release( );
}


void Device::ShowLastError( HRESULT inResult )
{
	LPTSTR aMsg;
	LPTSTR aTitle;
	DWORD aErr = GetLastError( );

	// Get Message
	FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, inResult, 0, ( LPTSTR ) &aMsg, 0, NULL );
	FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, aErr, 0, ( LPTSTR ) &aTitle, 0, NULL );

	//wsprintf( aTitle, TEXT( "%d" ), aErr );

	// Show Message
	MessageBox( NULL, aMsg, aTitle, MB_OK );
}

HRESULT Device::InitWindow( HINSTANCE hInstance, int nCmdShow )
{

	LPCSTR aName = "D3D10_Application";

	// Création / Enregistrement de la classe de la fenêtre
	WNDCLASSEX aClassEx;

	aClassEx.cbSize = sizeof( WNDCLASSEX );
	aClassEx.style = CS_HREDRAW | CS_VREDRAW;
	aClassEx.lpfnWndProc = &Device::WProc;
	aClassEx.cbClsExtra = 0;
	aClassEx.cbWndExtra = 0;
	aClassEx.hInstance = hInstance;
	aClassEx.hIcon = NULL;
	aClassEx.hCursor = LoadCursor( NULL, IDC_ARROW );
	aClassEx.hbrBackground = ( HBRUSH ) ( COLOR_WINDOW + 1 );
	aClassEx.lpszMenuName = NULL;
	aClassEx.lpszClassName = aName;
	aClassEx.hIconSm = NULL;

	if( !RegisterClassEx( &aClassEx ) )
	{
		return E_FAIL;
	}

	RECT aRect = { 0, 0, 600, 600 };
	AdjustWindowRect( &aRect, WS_OVERLAPPEDWINDOW, FALSE );

	m_hWnd = CreateWindow(
		aName,
		"Particles DirectX10",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		aRect.right - aRect.left,
		aRect.bottom - aRect.top,
		NULL,
		NULL,
		hInstance,
		this );

	if( !m_hWnd )
		return E_FAIL;

	ShowWindow( m_hWnd, nCmdShow );


	return S_OK;
}


HRESULT Device::InitDevice( )
{
	HRESULT aResult = S_OK;


	// Get viewport of the window
	GetClientRect( m_hWnd, &m_clientRect );
	m_width = m_clientRect.right - m_clientRect.left;
	m_height = m_clientRect.bottom - m_clientRect.top;

	// Set text mode
	m_hDC = GetDC( m_hWnd );
	SetTextColor( m_hDC, 0x00ffffff );
	SetBkMode( m_hDC, TRANSPARENT );
	// Change viewport for text offset (this rect will only be used for text drawing purpose)
	m_clientRect.left = 10;
	m_clientRect.top = 10;

	UINT aCreateDeviceFlags = 0;
#ifdef _DEBUG
	aCreateDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

	// Creation de la swapChain

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;

	ZeroMemory( &SwapChainDesc, sizeof( SwapChainDesc ) );

	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.BufferDesc.Width = m_width;
	SwapChainDesc.BufferDesc.Height = m_height;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = m_hWnd;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.Windowed = TRUE;

	// GPU Perf

#if SHIPPING_VERSION
	// When building a shipping version, disable PerfHUD (opt-out)
#else

	// Look for 'NVIDIA PerfHUD' adapter
	// If it is present, override default settings
	IDXGIFactory *pDXGIFactory;

	HRESULT hRes;
	hRes = CreateDXGIFactory( __uuidof( IDXGIFactory ), ( void** ) &pDXGIFactory );

	// Search for a PerfHUD adapter.
	UINT nAdapter = 0;
	IDXGIAdapter* aAdapter = NULL;
	IDXGIAdapter* aSelectedAdapter = NULL;
	D3D10_DRIVER_TYPE aDriverType = D3D10_DRIVER_TYPE_HARDWARE;
	while( pDXGIFactory->EnumAdapters( nAdapter, &aAdapter ) != DXGI_ERROR_NOT_FOUND )
	{
		if( aAdapter )
		{
			DXGI_ADAPTER_DESC adaptDesc;
			if( SUCCEEDED( aAdapter->GetDesc( &adaptDesc ) ) )
			{
				const bool isPerfHUD = wcscmp( adaptDesc.Description, L"NVIDIA PerfHUD" ) == 0;
				// Select the first adapter in normal circumstances or the PerfHUD one if it exists.
				if( nAdapter == 0 || isPerfHUD )
					aSelectedAdapter = aAdapter;
				if( isPerfHUD )
					aDriverType = D3D10_DRIVER_TYPE_REFERENCE;
			}
		}
		++nAdapter;
	}
#endif

	if( FAILED( aResult = D3D10CreateDeviceAndSwapChain(
		aSelectedAdapter,
		aDriverType,
		NULL,
		aCreateDeviceFlags,
		D3D10_SDK_VERSION,
		&SwapChainDesc,
		&m_SwapChain,
		&m_d3dDevice ) ) )
	{
		return aResult;
	}


	// Set viewport
	D3D10_VIEWPORT aNewVP;
	aNewVP.Width = m_width;
	aNewVP.Height = m_height;
	aNewVP.TopLeftX = 0;
	aNewVP.TopLeftY = 0;
	aNewVP.MinDepth = 0.0f;
	aNewVP.MaxDepth = 1.0f;

	m_d3dDevice->RSSetViewports( 1, &aNewVP );

	return aResult;
}


LRESULT CALLBACK Device::WProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	Device* aMySelf;

	// Get pointer to window
	if( message == WM_CREATE )
	{
		aMySelf = ( Device* ) ( ( LPCREATESTRUCT ) lParam )->lpCreateParams;
		SetWindowLongPtr( hWnd, GWLP_USERDATA, ( LONG_PTR ) aMySelf );
	}
	else
	{
		aMySelf = ( Device* ) GetWindowLongPtr( hWnd, GWLP_USERDATA );
		if( !aMySelf )
		{
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
	}

	aMySelf->m_hWnd = hWnd;
	return aMySelf->RealWProc( hWnd, message, wParam, lParam );


}

LRESULT CALLBACK Device::RealWProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	// En fonction du message, on execute diverses opération :
	switch( message )
	{

		// Fermeture de la fenêtre
	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;

	case WM_KEYDOWN:

		// A -> Up
		if( wParam == 'A' )
		{
			m_Particule->IncY( );
		}

		// E -> Down
		else if( wParam == 'E' )
		{
			m_Particule->DecY( );
		}

		// Q -> Left
		else if( wParam == 'Q' )
		{
			m_Particule->IncX( );
		}

		// D -> Right
		else if( wParam == 'D' )
		{
			m_Particule->DecX( );
		}

		// Z -> Forward
		else if( wParam == 'Z' )
		{
			m_Particule->IncZ( );
		}

		// S -> Backward
		else if( wParam == 'S' )
		{
			m_Particule->DecZ( );
		}

		break;

		// Default
	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}

	return 0;
}


void Device::Render( )
{
	m_Particule->render( );

	sprintf_s( m_TextBuffer, "Frame rate : %lf ms ; FPS : %d", m_Particule->getFrameRate( ), ( int ) ( 1000 / m_Particule->getFrameRate( ) ) );
	DrawText( m_hDC, m_TextBuffer, -1, &m_clientRect, DT_SINGLELINE | DT_NOCLIP );


	m_clientRect.top += 20;
	D3DXVECTOR3 aPos = m_Particule->getCameraPos();


	sprintf_s( m_TextBuffer, "X : %f Y : %f Z : %f", aPos.x, aPos.y, aPos.z );
	DrawText( m_hDC, m_TextBuffer, -1, &m_clientRect, DT_SINGLELINE | DT_NOCLIP );

	m_clientRect.top -= 20;
}