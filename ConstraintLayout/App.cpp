// Copyright Freiyer P. Kim, all rights reserved.

#include "stdafx.h"
#include "App.h"
#include "ConstraintView.h"

const AppDesc	*	App::_desc = nullptr;
const wchar_t		App::_className[] = L"ConstraintLayoutClass";
const wchar_t		App::_windowName[] = L"ConstraintLayout";
float				App::_dpiX = 0.0f;
float				App::_dpiY = 0.0f;
App				*	App::_instance = nullptr;

ID2D1Factory	*	App::_pD2DFactory = nullptr;
IDWriteFactory	*	App::_pDWriteFactory = nullptr;

int App::Run(_In_ const AppDesc * desc)
{
#define CHECK(exp)				{\
	OutputDebugStringA(#exp);	\
	OutputDebugStringA("\n");	\
	if(!(exp)) return 1;}

	_desc = desc;

	CHECK(_RegisterClass());
	CHECK(_InitializeFactories());
	CHECK(_CreateWindow());


	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	_DestroyWindow();
	_DestroyFactories();

	return msg.wParam;

#undef CHECK
}

bool App::_RegisterClass()
{
	WNDCLASSEX			_wcex;
	_wcex.cbSize = sizeof(_wcex);
	_wcex.style = CS_HREDRAW | CS_VREDRAW;
	_wcex.lpfnWndProc = _WindowProc;
	_wcex.cbClsExtra = 0;
	_wcex.cbWndExtra = 0;
	_wcex.hInstance = _desc->hInstance;
	_wcex.hIcon = LoadIcon(_desc->hInstance, MAKEINTRESOURCE(IDI_CONSTRAINTLAYOUT));
	_wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	_wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	_wcex.lpszMenuName = NULL;
	_wcex.lpszClassName = _className;
	_wcex.hIconSm = LoadIcon(_desc->hInstance, MAKEINTRESOURCE(IDI_SMALL));
	if (!RegisterClassEx(&_wcex))
		return false;

	return true;
}

bool App::_InitializeFactories()
{
#define CHECK(exp)				{\
	OutputDebugStringA(#exp);	\
	OutputDebugStringA("");		\
	if(FAILED(exp))				\
		return false;			}

	CHECK(D2D1CreateFactory(
		D2D1_FACTORY_TYPE_MULTI_THREADED,
		&_pD2DFactory
	));
	_pD2DFactory->GetDesktopDpi(&_dpiX, &_dpiY);

	CHECK(DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(_pDWriteFactory),
		reinterpret_cast<IUnknown **>(&_pDWriteFactory)
	));

	return true;

#undef CHECK
}

bool App::_CreateWindow()
{
	if (_instance)
		return false;

	_instance = new ConstraintView;
	if (!_instance->OnCreate())
	{
		delete _instance;
		_instance = nullptr;
		return false;
	}
	return true;
}

void App::_DestroyWindow()
{
	if (_instance)
	{
		delete _instance;
		_instance = nullptr;
	}
}

void App::_DestroyFactories()
{
	SafeRelease(&_pDWriteFactory);
	SafeRelease(&_pD2DFactory);
}

LRESULT App::_WindowProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	App * app = reinterpret_cast<App *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	// Check mouse input
	static auto const iMsg2Mouse = [](UINT iMsg) -> MouseCommands { switch (iMsg) {
#define CASE(name) case WM_##name: return MOUSE_##name

		CASE(LBUTTONUP); CASE(LBUTTONDOWN); CASE(LBUTTONDBLCLK);
		CASE(RBUTTONUP); CASE(RBUTTONDOWN); CASE(RBUTTONDBLCLK);

#undef CASE	
	} return MOUSE_UNKNOWN; };

	if (auto cmd = iMsg2Mouse(iMsg); app && cmd != MOUSE_UNKNOWN)
		app->OnMouse(
			GET_X_LPARAM(lParam),
			GET_Y_LPARAM(lParam),
			iMsg2Mouse(iMsg)
		);

	// Handle other events
	switch (iMsg)
	{
	case WM_CREATE:
		SetWindowLongPtr(hWnd, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(
				reinterpret_cast<LPCREATESTRUCT>(lParam)
				->lpCreateParams
				)
		);
		return 0;
	case WM_KEYDOWN:
		if (app)
			app->OnKeyDown(static_cast<UINT8>(wParam));
		return 0;
	case WM_KEYUP:
		if (app)
			app->OnKeyUp(static_cast<UINT8>(wParam));
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		if (app)
		{
			if (!app->OnUpdate())
				PostQuitMessage(0);
			if (!app->OnRender())
				PostQuitMessage(0);
		}
		return 0;
	case WM_SIZE:
		if(app)
			app->OnSizeChange(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}


std::wstring ToWideChar(const char * str, int len)
{
	if (len == 0)
		return std::wstring();
	int num = MultiByteToWideChar(
		CP_UTF8, 0, str, len, NULL, 0
	);
	std::wstring rtn(num, 0);
	MultiByteToWideChar(
		CP_UTF8, 0, str, len, &rtn[0], num
	);
	return rtn;
}

std::wstring ToWideChar(const std::string & str)
{
	return ToWideChar(str.c_str(), (int)str.size());
}

std::string FromWideChar(const wchar_t * str, int len)
{
	if (len == 0)
		return std::string();
	int num = WideCharToMultiByte(
		CP_UTF8, 0, str, len, NULL, 0, NULL, NULL
	);
	std::string rtn(num, 0);
	WideCharToMultiByte(
		CP_UTF8, 0, str, len, &rtn[0], num, NULL, NULL
	);
	return rtn;
}

std::string FromWideChar(const std::wstring & str)
{
	return FromWideChar(str.c_str(), (int)str.size());
}
