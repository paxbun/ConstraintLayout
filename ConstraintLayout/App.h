#pragma once
#include "stdafx.h"

struct AppDesc
{
	HINSTANCE		hInstance;
	int				nCmdShow;
	int				scrnWidth;
	int				scrnHeight;
};

class App
{
public:
	static int Run(_In_ const AppDesc * desc);

private:
	static bool _RegisterClass();
	static bool _InitializeFactories();
	static bool _CreateWindow();
	static void _DestroyWindow();
	static void _DestroyFactories();

	static LRESULT WINAPI _WindowProc(
		_In_	HWND	hWnd,
		_In_	UINT	iMsg,
		_In_	WPARAM	wParam,
		_In_	LPARAM	lParam
	);

	static App			 *		_instance;

protected:

	static const AppDesc *		_desc;
	static const wchar_t		_className[];
	static const wchar_t		_windowName[];

	static ID2D1Factory		*	_pD2DFactory;
	static IDWriteFactory	*	_pDWriteFactory;
	static float				_dpiX;
	static float				_dpiY;

public:

	enum MouseCommands
	{
		MOUSE_LBUTTONUP, MOUSE_LBUTTONDOWN, MOUSE_LBUTTONDBLCLK,
		MOUSE_RBUTTONUP, MOUSE_RBUTTONDOWN, MOUSE_RBUTTONDBLCLK,
		MOUSE_UNKNOWN
	};

	virtual bool OnCreate() = 0;
	virtual void OnKeyDown(UINT8 key) = 0;
	virtual void OnKeyUp(UINT8 key) = 0;
	virtual void OnMouse(SHORT x, SHORT y, MouseCommands cmd) = 0;
	virtual bool OnSizeChange(WORD x, WORD y) = 0;
	virtual bool OnUpdate() = 0;
	virtual bool OnRender() = 0;
	virtual void OnDestroy() = 0;

};

template<typename T>
inline void SafeRelease(T ** intf)
{
	if (*intf)
	{
		(*intf)->Release();
		*intf = NULL;
	}
}

std::wstring ToWideChar(const char * str, int len);
std::wstring ToWideChar(const std::string & str);
std::string FromWideChar(const wchar_t * str, int len);
std::string FromWideChar(const std::wstring & str);