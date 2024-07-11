#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include "d3dx12.h"

#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3dcompiler")


#include <wrl.h>


typedef unsigned int uint;

using Microsoft::WRL::ComPtr;

#include <stdio.h>
#include <iostream>

#define HR(x) __hr(x, __FILE__, __LINE__)

#define safe_delete(x) if(x) { delete x; x = nullptr; }
#define safe_release(x) if(x) { x->Release(); x = nullptr; }

inline void __hr(HRESULT hr, LPCSTR filename, int line)
{
	
	if (FAILED(hr)) {
		char* buffer = {};
		FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,hr,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
			(LPSTR)&buffer,0,NULL
		);
		LocalFree(buffer);
		char totalCuffer[256];
		filename = (::strrchr(filename,'\\') + 1);
		sprintf_s(totalCuffer,"%s\nfile: %s\n line %d\n",buffer,filename,line);

		if (MessageBoxA(NULL, totalCuffer, "HRESULT Error", MB_OK)) {
			ExitProcess(0);
		}

	}
}




