// Copyright Freiyer P. Kim, all rights reserved.

#include "stdafx.h"
#include "App.h"

int WINAPI wWinMain(
	_In_		HINSTANCE	hInstance,
	_In_opt_	HINSTANCE	hPrevInstance,
	_In_		LPWSTR		lpCmdLine,
	_In_		int			nCmdShow
)
{
	// Fill the description table
	AppDesc desc;
	{
		desc.hInstance = hInstance;
		desc.nCmdShow = nCmdShow;
		desc.scrnWidth = 1280;// GetSystemMetrics(SM_CXSCREEN);
		desc.scrnHeight = 720;// GetSystemMetrics(SM_CXSCREEN);
	}
	
	return App::Run(&desc);
}