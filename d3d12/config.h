#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d3d12.h>
//#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>
#include "d3dx12.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3DCompiler.lib")


#include <wrl.h>


typedef unsigned int uint;

using Microsoft::WRL::ComPtr;

template<typename T>
using com_ptr = Microsoft::WRL::ComPtr<T>;




#include <stdio.h>
#include <iostream>

#define HR(x) __hr(x, __FILE__, __LINE__)

#define safe_delete(x) if(x) { delete x; x = nullptr; }
#define safe_release(x) if(x) { x->Release(); x = nullptr; }

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
	#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#ifdef _DEBUG
#define set_name(obj, name) if(obj) { obj->SetName(L#name);}
#else
#define set_name(ovj, name)
#endif

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




