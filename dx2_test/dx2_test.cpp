
#define WINVER 0x0400
#define _WIN32_WINNT 0x0400

#define INITGUID
#define DIRECTDRAW_VERSION 0x0200
#include <ddraw.h>

#if 0
#define DIRECT3D_VERSION 0x0200

#if(DIRECT3D_VERSION < 0x0500)
#define D3DCOLOR_MONO   1
#define D3DCOLOR_RGB    2
typedef DWORD D3DCOLORMODEL;

#define D3DERR_VIEWPORTDATANOTSET       MAKE_DDHRESULT(773)
#define D3DERR_VIEWPORTHASNODEVICE      MAKE_DDHRESULT(774)
#endif
#endif

#include <d3d.h>
#include "d3dmacs.h"
#include <stdio.h>
//#include <comdef.h>
//#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "ddraw.lib")

//#pragma comment(lib, "dxguid.lib")

struct TestContext
{
	LPCTSTR class_name = TEXT("DXTestWin");
	HWND window;
	bool fullscreen = false;
	bool shouldClose = false;
	IDirectDraw *pdd = nullptr;
	IDirectDraw2 *pdd2 = nullptr;
	IDirect3D *pd3d = nullptr;
	IDirectDrawSurface *pddsPrimary = nullptr;
	IDirectDrawSurface *pddsBack = nullptr;
	IDirectDrawSurface *pdds1 = nullptr;
	IDirectDrawClipper *pddClipper = nullptr;
	IDirect3DDevice *pd3dd = nullptr;
	IDirect3DViewport *pd3dViewport = nullptr;
	IDirect3DExecuteBuffer *pd3deb = nullptr;

	IDirectDrawSurface *pddsTex = nullptr;
	IDirect3DTexture *pd3dTex = nullptr;
	D3DTEXTUREHANDLE texHandle = 0;

	D3DMATRIXHANDLE mthWorld = 0;

	bool initWin(int w, int h, bool fullScreen);
	int initDD();
	int initD3D();
	void loop();
	void draw();
	void onWinDestroy();
	void shutdownWin();
	void shutdownDD();
	void shutdownD3D();

	HRESULT d3dSetState(D3DOPCODE op, int state, DWORD value);
};

const char *WinProcMsgToString(UINT uMsg)
{
	switch (uMsg)
	{
	case WM_CREATE: return "WM_CREATE";
	case WM_DESTROY: return "WM_DESTROY";
	case WM_MOVE: return "WM_MOVE";
	case WM_SIZE: return "WM_SIZE";
	case WM_ACTIVATE: return "WM_ACTIVATE";
	case WM_SETFOCUS: return "WM_SETFOCUS";
	case WM_KILLFOCUS: return "WM_KILLFOCUS";

	case WM_PAINT: return "WM_PAINT";
	case WM_CLOSE: return "WM_CLOSE";

	case WM_QUERYOPEN: return "WM_QUERYOPEN";
	case WM_ERASEBKGND: return "WM_ERASEBKGND";

	case WM_SHOWWINDOW: return "WM_SHOWWINDOW";

	case WM_ACTIVATEAPP: return "WM_ACTIVATEAPP";

	case WM_SETCURSOR: return "WM_SETCURSOR";
	case WM_MOUSEACTIVATE: return "WM_MOUSEACTIVATE";

	case WM_GETMINMAXINFO: return "WM_GETMINMAXINFO";

	case WM_WINDOWPOSCHANGING: return "WM_WINDOWPOSCHANGING";
	case WM_WINDOWPOSCHANGED: return "WM_WINDOWPOSCHANGED";

	case WM_GETICON: return "WM_GETICON";

	case WM_NCCREATE: return "WM_NCCREATE";
	case WM_NCDESTROY: return "WM_NCDESTROY";
	case WM_NCCALCSIZE: return "WM_NCCALCSIZE";
	case WM_NCHITTEST: return "WM_NCHITTEST";
	case WM_NCPAINT: return "WM_NCPAINT";
	case WM_NCACTIVATE: return "WM_NCACTIVATE";

	case WM_NCMOUSEMOVE: return "WM_NCMOUSEMOVE";
	case WM_NCLBUTTONDOWN: return "WM_NCLBUTTONDOWN";

	case WM_NCLBUTTONDBLCLK: return "WM_NCLBUTTONDBLCLK";

	case WM_SYSCOMMAND: return "WM_SYSCOMMAND";

	case WM_MOUSEMOVE: return "WM_MOUSEMOVE";
	case WM_LBUTTONDOWN: return "WM_LBUTTONDOWN";
	case WM_LBUTTONUP: return "WM_LBUTTONUP";

	case WM_SIZING: return "WM_SIZING";
	case WM_CAPTURECHANGED: return "WM_CAPTURECHANGED";

	case WM_ENTERSIZEMOVE: return "WM_ENTERSIZEMOVE";

	case WM_IME_SETCONTEXT: return "WM_IME_SETCONTEXT";
	case WM_IME_NOTIFY: return "WM_IME_NOTIFY";

#if(WINVER >= 0x0500)
	case WM_NCMOUSELEAVE: return "WM_NCMOUSELEAVE";
#endif

#if(_WIN32_WINNT >= 0x0600)
	case WM_DWMNCRENDERINGCHANGED: return "WM_DWMNCRENDERINGCHANGED";
#endif
	}
	return "?";
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#if 0
	static int paints = 0;
	if (uMsg == WM_PAINT)
		paints++;

	if (uMsg != WM_PAINT || paints < 20)
		printf("WindowProc(%p, %X(%s), %p, %p)\n", hWnd, uMsg, WinProcMsgToString(uMsg), wParam, lParam);
#endif

	TestContext *ctx = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		ctx = (TestContext *)((CREATESTRUCT *)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)ctx);
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	ctx = (TestContext *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (uMsg) {
#if 1
#if 0
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO *info = (MINMAXINFO *)lParam;
		printf("MINMAXINFO ptMaxSize (%d,%d) ptMaxPosition (%d,%d) ptMinTrackSize (%d,%d) ptMaxTrackSize (%d,%d) \n",
			info->ptMaxSize.x, info->ptMaxSize.y, info->ptMaxPosition.x, info->ptMaxPosition.y,
			info->ptMinTrackSize.x, info->ptMinTrackSize.y, info->ptMaxTrackSize.x, info->ptMaxTrackSize.y);
		info->ptMinTrackSize.x = 256;
		info->ptMinTrackSize.y = 128;
		info->ptMaxTrackSize.x = 1024;
		info->ptMaxTrackSize.y = 1024;
		return 0;
	}
#endif
	case WM_NCCALCSIZE:
	{
		if (!wParam)
		{
			RECT *r = (RECT *)lParam;
			printf("NCCALCSIZE %d %d %d %d\n", r->left, r->top, r->right, r->bottom);
		}
		break;
	}
#if 0
	case WM_WINDOWPOSCHANGING:
	case WM_WINDOWPOSCHANGED:
	{
		WINDOWPOS *info = (WINDOWPOS *)lParam;
		printf("WINDOWPOS %p %p (%d %d %d %d) %X(",
			info->hwnd, info->hwndInsertAfter, info->x, info->y, info->cx, info->cy, info->flags);
		if (info->flags & SWP_NOSIZE)
			printf("NOSIZE ");
		if (info->flags & SWP_NOMOVE)
			printf("NOMOVE ");
		if (info->flags & SWP_NOZORDER)
			printf("NOZORDER ");
		if (info->flags & SWP_NOACTIVATE)
			printf("NOACTIVATE ");
		if (info->flags & SWP_FRAMECHANGED)
			printf("FRAMECHANGED ");
		if (info->flags & SWP_SHOWWINDOW)
			printf("SHOWWINDOW ");
		if (info->flags & SWP_HIDEWINDOW)
			printf("HIDEWINDOW ");
		if (info->flags & SWP_NOCOPYBITS)
			printf("NOCOPYBITS ");
		printf(")\n");
		//if (uMsg == WM_WINDOWPOSCHANGED)
		//	return 0;//to skip WM_SIZE and WM_MOVE
		break;
	}
	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);
		const char *stypes[]{"SIZE_RESTORED","SIZE_MINIMIZED","SIZE_MAXIMIZED","SIZE_MAXSHOW","SIZE_MAXHIDE"};
		printf("SIZE %X(%s) %dx%d\n", wParam, (wParam >= 0 && wParam <= 4) ? stypes[wParam] : "?", width, height);
		break;
	}
	case WM_MOVE:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		printf("MOVE %dx%d\n", x, y);
		break;
	}
#endif
	//case WM_CREATE:
	//	return 0;
	//case WM_SHOWWINDOW:
	// wParam isShown
	//	break
	//case WM_ACTIVATEAPP:
		//wParam isActivated
	//	break;
	//case WM_ERASEBKGND:
		//HDC dc = (HDC)wParam;
	//	return 1;
	case WM_DESTROY:
		if (ctx)
			ctx->onWinDestroy();
		PostQuitMessage(0);
		return 0;

	case WM_TIMER:
	{
		//if (ctx)
		//	ctx->draw();
		break;
	}
#if 1
	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		HDC dc = BeginPaint(hWnd, &paint);
		if (!dc)
			return 0;

		//if (ctx)
		//	ctx->draw();
		BOOL r = EndPaint(hWnd, &paint);
		return 0;
	}
#endif
#endif
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool TestContext::initWin(int w, int h, bool fs)
{
	fullscreen = fs;
	printf("initWin(%d,%d)\n", w, h);

	DWORD style = WS_OVERLAPPEDWINDOW;//WS_OVERLAPPED | WS_SYSMENU // | WS_VISIBLE
	if (fullscreen)
		style = WS_POPUP;
	DWORD exStyle = WS_EX_APPWINDOW;
	RECT winSize{ 0,0,w,h };
	BOOL r = AdjustWindowRectEx(&winSize, style, false, exStyle);
	printf("%d AdjustWindowRectEx %d %d %d %d\n", r, winSize.left, winSize.top, winSize.right, winSize.bottom);

	HINSTANCE inst = GetModuleHandle(nullptr);
	//WNDCLASS wndClass{ CS_HREDRAW | CS_VREDRAW | CS_OWNDC, WindowProc, 0, 0,
	//	inst, nullptr, nullptr, nullptr, nullptr, class_name };
	WNDCLASS wndClass{};
	wndClass.style = CS_HREDRAW | CS_VREDRAW;// | CS_OWNDC;
	wndClass.lpfnWndProc = WindowProc;
	wndClass.hInstance = inst;
	//wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszClassName = class_name;
	ATOM wc = RegisterClass(&wndClass);
	printf("RegisterClass = %X\n", wc);
	if (!wc)
		return true;
	window = CreateWindowEx(exStyle, class_name, TEXT("DX test"), style | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, winSize.right - winSize.left, winSize.bottom - winSize.top, nullptr, nullptr, inst, this);
	printf("CreateWindowEx = %p\n", window);
	if (!window)
		return true;
	//r = ShowWindow(window, SW_SHOW);
	//printf("ShowWindow = %d\n", r);
	//r = UpdateWindow(window);
	//printf("UpdateWindow = %d\n", r);

	return false;
}

void TestContext::onWinDestroy()
{
	window = nullptr;
	shutdownD3D();
	shutdownDD();
	shouldClose = true;
}

void TestContext::shutdownWin()
{
	if (window)
		DestroyWindow(window);
	HINSTANCE inst = GetModuleHandle(nullptr);
	UnregisterClass(class_name, inst);
}


std::string ddResultToStr(HRESULT r)
{
	if (r == DD_OK)
		return "DD_OK";
	if (r == DDERR_EXCEPTION)
		return "DDERR_EXCEPTION";
	if (r == DDERR_INVALIDPARAMS)
		return "DDERR_INVALIDPARAMS";
	if (r == DDERR_INVALIDOBJECT)
		return "DDERR_INVALIDOBJECT";
	if (r == DDERR_UNSUPPORTED)
		return "DDERR_UNSUPPORTED";
	if (r == DDERR_NOCOOPERATIVELEVELSET)
		return "DDERR_NOCOOPERATIVELEVELSET";
	if (r == DDERR_INVALIDCAPS)
		return "DDERR_INVALIDCAPS";
	if (r == DDERR_INCOMPATIBLEPRIMARY)
		return "DDERR_INCOMPATIBLEPRIMARY";
	if (r == DDERR_NOEXCLUSIVEMODE)
		return "DDERR_NOEXCLUSIVEMODE";
	if (r == DDERR_NOFLIPHW)
		return "DDERR_NOFLIPHW";
	if (r == DDERR_PRIMARYSURFACEALREADYEXISTS)
		return "DDERR_PRIMARYSURFACEALREADYEXISTS";
	if (r == DDERR_UNSUPPORTEDMODE)
		return "DDERR_UNSUPPORTEDMODE";
	if (r == DDERR_NOTFOUND)
		return "DDERR_NOTFOUND";
#if (DIRECT3D_VERSION >= 0x0500)
	if (r == D3DERR_VIEWPORTDATANOTSET)
		return "D3DERR_VIEWPORTDATANOTSET";
	if (r == D3DERR_VIEWPORTHASNODEVICE)
		return "D3DERR_VIEWPORTHASNODEVICE";
#endif

	//return std::to_string(r);

#if 0
	_com_error err(r);
	LPCTSTR errMsg = err.ErrorMessage();
	wprintf(L"err %ls\n", errMsg);
	std::wcout << errMsg << std::endl;
#endif
#if 1
	//Return "Not implemented" for DDERR_UNSUPPORTED
	LPSTR lpMsgBuf = nullptr;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		r,
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf,
		0, NULL);

	//printf("error %d %s\n", r, lpMsgBuf);
	if (!lpMsgBuf)
	{
		int sev = (r >> 31) & 1;
		int fac = (r >> 16) & 0x7FFF;
		int code = r & 0xFFFF;
		char buff[256]{};
		snprintf(buff, 255, "%d 0x%X %d", sev, fac, code);
		return std::string(buff);
	}
	std::string ret(lpMsgBuf);
	LocalFree(lpMsgBuf);
	return ret;
#endif
}


void TestContext::loop()
{
	if (!SetTimer(window, 1, 10, NULL))
		return;

	while (!shouldClose)
	{
		MSG msg{};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (!pddsPrimary)
			continue;

		//InvalidateRect(window, nullptr, false);
		//draw();

		HWND win = window;
		int winWidth = 640;
		int winHeight = 480;
		RECT dstRect{};
		GetClientRect(win, &dstRect);
		POINT point{ 0,0 };
		ClientToScreen(win, &point);
		OffsetRect(&dstRect, point.x, point.y);

		RECT srcRect{ 0,0, winWidth, winHeight };
		//printRect("src", srcRect);
		//printRect("dst", dstRect);
		HRESULT r = pddsPrimary->Blt(&dstRect, pddsBack, &srcRect, DDBLT_WAIT, nullptr);

		DDBLTFX bltFx{ 0 };
		bltFx.dwSize = sizeof(bltFx);
		bltFx.dwFillColor = RGB(127, 50, 70);
		//HRESULT r = pddsPrimary->Blt(&dstRect, nullptr, nullptr, DDBLT_WAIT | DDBLT_COLORFILL, &bltFx);
		if(r)
			printf("%d (%s) Blt\n", r, ddResultToStr(r).c_str());
	}
}

template<typename T>
int VerboseRelease(T *&obj, const char *msg)
{
	if (!obj)
		return 0;
	int c = obj->Release();
	obj = nullptr;
	printf("%d %s\n", c, msg);
	return c;
}

void printDDSDesc(const DDSURFACEDESC &ddsd)
{
	printf(" size %d flags %X", ddsd.dwSize, ddsd.dwFlags);
	if (ddsd.dwFlags & DDSD_CAPS)
	{
		printf(" caps %X(", ddsd.ddsCaps.dwCaps);
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER)
			printf("BACKBUFFER ");
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_COMPLEX)
			printf("COMPLEX ");
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_FLIP)
			printf("FLIP ");
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER)
			printf("FRONTBUFFER ");
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_OFFSCREENPLAIN)
			printf("OFFSCREENPLAIN ");
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
			printf("PRIMARYSURFACE ");
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
			printf("SYSTEMMEMORY ");
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_TEXTURE)
			printf("TEXTURE ");
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_3DDEVICE)
			printf("3DDEVICE ");
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
			printf("VIDEOMEMORY ");
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_VISIBLE)
			printf("VISIBLE ");
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_MIPMAP)
			printf("MIPMAP ");
#if (DIRECTDRAW_VERSION >= 0x0500)
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_LOCALVIDMEM)
			printf("LOCALVIDMEM ");
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_STANDARDVGAMODE)
			printf("SVGAMODE ");
#endif
		printf(")");
	}
#if 0
	if (ddsd.dwFlags & DDSD_WIDTH)
		printf("  width %d\n", ddsd.dwWidth);
	if (ddsd.dwFlags & DDSD_HEIGHT)
		printf("  height %d\n", ddsd.dwHeight);
	if (ddsd.dwFlags & DDSD_REFRESHRATE)
		printf("  rate %d\n", ddsd.dwRefreshRate);
#else
	if (ddsd.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT))
		printf(" res %dx%d", ddsd.dwWidth, ddsd.dwHeight);
	if (ddsd.dwFlags & DDSD_REFRESHRATE)
		printf(" rate %d", ddsd.dwRefreshRate);
#endif
	if (ddsd.dwFlags & DDSD_PITCH)
		printf(" pitch %d", ddsd.lPitch);
	if (ddsd.dwFlags & DDSD_BACKBUFFERCOUNT)
		printf(" backbuffers %d", ddsd.dwBackBufferCount);
	if (ddsd.dwFlags & DDSD_MIPMAPCOUNT)
		printf(" mipmaps %d", ddsd.dwMipMapCount);
	putc('\n', stdout);
	if (ddsd.dwFlags & DDSD_PIXELFORMAT)
	{
		const DDPIXELFORMAT &pf = ddsd.ddpfPixelFormat;
		printf("  pixelformat size %d flags %X", pf.dwSize, pf.dwFlags);
		if (pf.dwFlags & DDPF_RGB)
		{
			if (pf.dwRGBBitCount == 32 && pf.dwRBitMask == 0xFF0000 && pf.dwGBitMask == 0xFF00 && pf.dwBBitMask == 0xFF)
				if (pf.dwFlags & DDPF_ALPHAPIXELS)
					printf(" BGRA8");
				else
					printf(" BGR8");
			else if (pf.dwFlags & DDPF_ALPHAPIXELS && pf.dwRGBBitCount == 16 && pf.dwRBitMask == 0xF00 && pf.dwGBitMask == 0xF0 && pf.dwBBitMask == 0xF)
				printf(" BGRA4");
			else if (pf.dwRGBBitCount == 16 && pf.dwRBitMask == 0x7C00 && pf.dwGBitMask == 0x3E0 && pf.dwBBitMask == 0x1F)
				if (pf.dwFlags & DDPF_ALPHAPIXELS)
					printf(" BGR5A1");
				else
					printf(" BGR5");
			else if (pf.dwRGBBitCount == 16 && pf.dwRBitMask == 0xF800 && pf.dwGBitMask == 0x7E0 && pf.dwBBitMask == 0x1F)
				printf(" B5G6R5");
			else
				printf(" RGB");

			printf(" bits %d r %X g %X b %X", pf.dwRGBBitCount, pf.dwRBitMask, pf.dwGBitMask, pf.dwBBitMask);
		}

		if (pf.dwFlags & DDPF_ALPHAPIXELS)
			printf(" a %X", pf.dwRGBAlphaBitMask);
		if (pf.dwFlags & DDPF_RGB)
			printf("\n");
		if (pf.dwFlags & DDPF_FOURCC)
			printf("   FourCC \"%.4s\"\n", (const char *)&pf.dwFourCC);
	}
}

struct ddEnumContext_t
{
	int count = 0;
	GUID selected{};
	IDirectDraw *pdd = nullptr;
};

#if !defined(DIRECTDRAW_VERSION) || DIRECTDRAW_VERSION <= 0x0200
#define DDCAPS_DX1 DDCAPS
#endif

BOOL WINAPI ddEnumCallbackA(GUID FAR *lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext)
{
	printf("ddEnumCallback");
	printf(" guid %p desc \"%s\" name \"%s\" context %p\n", lpGUID, lpDriverDescription, lpDriverName, lpContext);
	ddEnumContext_t *ctx = (ddEnumContext_t *)lpContext;
	ctx->count++;
	if (lpGUID)
	{
		IDirectDraw *pdd = nullptr;
		HRESULT	r = DirectDrawCreate(lpGUID, &pdd, nullptr);
		printf("%d (%s) DirectDrawCreate %p\n", r, ddResultToStr(r).c_str(), pdd);
		if (r != DD_OK)
			return DDENUMRET_OK;

		DDCAPS_DX1 driverCaps{};
		driverCaps.dwSize = sizeof(driverCaps);
		DDCAPS_DX1 HELCaps{};
		HELCaps.dwSize = sizeof(HELCaps);

		r = pdd->GetCaps((DDCAPS *)&driverCaps, (DDCAPS *)&HELCaps);
		printf("%d IDirectDraw::GetCaps\n", r);
		if (r != DD_OK)
			return DDENUMRET_OK;
		printf(" driver caps %X ddsc %X, vmem %u, FCCs %d\n", driverCaps.dwCaps, driverCaps.ddsCaps.dwCaps, driverCaps.dwVidMemTotal, driverCaps.dwNumFourCCCodes);
		printf(" HEL caps %X ddsc %X, vmem %u, FCCs %d\n", HELCaps.dwCaps, HELCaps.ddsCaps.dwCaps, HELCaps.dwVidMemTotal, HELCaps.dwNumFourCCCodes);

		if (!ctx->pdd && (driverCaps.dwCaps & DDCAPS_3D))
		{
			if (lpGUID)
				ctx->selected = *lpGUID;
			ctx->pdd = pdd;
			//return DDENUMRET_CANCEL;
		}
		else
			VerboseRelease(pdd, "IDirectDraw::Release");
	}
	return DDENUMRET_OK;
}

#if 0
BOOL WINAPI ddEnumCallbackW(GUID FAR *lpGUID, LPWSTR lpDriverDescription, LPWSTR lpDriverName, LPVOID lpContext)
{
	printf("ddEnumCallback");
	printf(" guid %p desc \"%ls\" name \"%ls\" context %p\n", lpGUID, lpDriverDescription, lpDriverName, lpContext);
	ddEnumContext_t *ctx = (ddEnumContext_t *)lpContext;
	ctx->count++;
	return DDENUMRET_OK;
}

BOOL WINAPI ddEnumExCallback(GUID FAR *lpGUID, LPWSTR lpDriverDescription, LPWSTR lpDriverName, LPVOID lpContext, HMONITOR hm)
{
	printf("ddEnumCallback");
	printf(" guid %p desc \"%ls\" name \"%ls\" context %p monitor %p\n", lpGUID, lpDriverDescription, lpDriverName, lpContext, hm);
	ddEnumContext_t *ctx = (ddEnumContext_t *)lpContext;
	ctx->count++;
	return DDENUMRET_OK;
}
#endif

HRESULT WINAPI ddEnumModesCallback(LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
{
	printf(" desc %p context %p\n", lpDDSurfaceDesc, lpContext);
	if (lpDDSurfaceDesc)
		printDDSDesc(*lpDDSurfaceDesc);
	((int *)lpContext)[0]++;
	return DDENUMRET_OK;
}

HRESULT WINAPI ddEnumSurfacesCallback(LPDIRECTDRAWSURFACE lpDDSurface, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
{
	printf(" surf %p desc %p context %p\n", lpDDSurface, lpDDSurfaceDesc, lpContext);
	if (lpDDSurfaceDesc)
		printDDSDesc(*lpDDSurfaceDesc);

	VerboseRelease(lpDDSurface, "IDirectDrawSurface::Release Enum");
	//if (lpDDSurface)
	//	lpDDSurface->Release();
	((int *)lpContext)[0]++;
	return DDENUMRET_OK;
}

HRESULT WINAPI d3dEnumDevicesCallback(GUID FAR *lpGuid, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC lpD3DHWDeviceDesc, LPD3DDEVICEDESC lpD3DHELDeviceDesc, LPVOID lpUserArg)
{
	printf(" guid %X desc \"%s\" name \"%s\" hwDesc %p helDesc %p user %p\n", lpGuid->Data1, lpDeviceDescription, lpDeviceName, lpD3DHWDeviceDesc, lpD3DHELDeviceDesc, lpUserArg);
	printf("  flags %X %X, color model hw %d hel %d, MaxBufferSize hw %d hel %d\n", 
		lpD3DHWDeviceDesc->dwFlags, lpD3DHELDeviceDesc->dwFlags,
		lpD3DHWDeviceDesc->dcmColorModel, lpD3DHELDeviceDesc->dcmColorModel,
		lpD3DHWDeviceDesc->dwMaxBufferSize, lpD3DHELDeviceDesc->dwMaxBufferSize);
	((int *)lpUserArg)[0]++;
	return DDENUMRET_OK;
}

HRESULT WINAPI d3dEnumTexFmtCallback(LPDDSURFACEDESC lpDdsd, LPVOID lpContext)
{
	int &count = *(int *)lpContext;
	printf(" texFmt %d:", count);
	if (lpDdsd)
		printDDSDesc(*lpDdsd);
	count++;
	return DDENUMRET_OK;
}

int TestContext::initDD()
{
	HRESULT r;
	ddEnumContext_t ddEnumCtx;

	r = DirectDrawEnumerateA(ddEnumCallbackA, &ddEnumCtx);
	printf("%d (%s) DirectDrawEnumerateA %d\n", r, ddResultToStr(r).c_str(), ddEnumCtx.count);
	if (r != DD_OK)
		return -1;

#if 0
	HMODULE ddrawdll = LoadLibraryW(L"ddraw.dll");
	printf("LoadLibraryW(L\"ddraw.dll\") = %p\n", ddrawdll);
	if (ddrawdll)
	{
#if 0
		typedef HRESULT(WINAPI *LPDIRECTDRAWENUMERATEW)(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext);
		LPDIRECTDRAWENUMERATEW pddEnumW = (LPDIRECTDRAWENUMERATEW)GetProcAddress(ddrawdll, "DirectDrawEnumerateW");
		printf("proc DirectDrawEnumerateW = %p\n", pddEnumW);
		if (pddEnumW)
		{
			ddEnumCtx = {};
			HRESULT r = pddEnumW(ddEnumCallbackW, &ddEnumCtx);
			printf("%d DirectDrawEnumerateW %d\n", r, ddEnumCtx.count);
			if (r != DD_OK)
			{
				printf("err %s\n", ddResultToStr(r).c_str());
				return -1;
			}
		}
#endif
		LPDIRECTDRAWENUMERATEEXW pddEnumExW = (LPDIRECTDRAWENUMERATEEXW)GetProcAddress(ddrawdll, "DirectDrawEnumerateExW");
		printf("proc DirectDrawEnumerateExW = %p\n", pddEnumExW);
		if (pddEnumExW)
		{
			ddEnumCtx = {};
			HRESULT r = pddEnumExW(ddEnumExCallback, &ddEnumCtx, 0);
			printf("%d DirectDrawEnumerateW %d\n", r, ddEnumCtx.count);
			if (r != DD_OK)
			{
				printf("err %s\n", ddResultToStr(r).c_str());
				return -1;
			}
		}
	}
#endif
#if 0
	ddEnumCtx = {};
	r = DirectDrawEnumerateExW(ddEnumExCallback, &ddEnumCtx, 0);
	printf("%d(%s) DirectDrawEnumerateExW %d\n", r, ddResultToStr(r).c_str(), ddEnumCtx.count);
	if (r != DD_OK)
	{
		return -1;
	}
#endif

	if (!pdd)
	{
		r = DirectDrawCreate(nullptr, &pdd, nullptr);
		printf("%d (%s) DirectDrawCreate %p\n", r, ddResultToStr(r).c_str(), pdd);
		if (r != DD_OK)
			return -1;
	}

	DDCAPS_DX1 driverCaps{};
	driverCaps.dwSize = sizeof(driverCaps);
	DDCAPS_DX1 HELCaps{};
	HELCaps.dwSize = sizeof(HELCaps);

	r = pdd->GetCaps((DDCAPS *)&driverCaps, (DDCAPS *)&HELCaps);
	printf("%d IDirectDraw::GetCaps\n", r);
	printf(" driver caps %X ddsc %X, vmem %u, FCCs %d\n", driverCaps.dwCaps, driverCaps.ddsCaps.dwCaps, driverCaps.dwVidMemTotal, driverCaps.dwNumFourCCCodes);
	printf(" HEL caps %X ddsc %X, vmem %u, FCCs %d\n", HELCaps.dwCaps, HELCaps.ddsCaps.dwCaps, HELCaps.dwVidMemTotal, HELCaps.dwNumFourCCCodes);

	SIZE renderSize{};
	RECT dstRect{};
	if (window)
	{
		POINT point{ 0,0 };
		ClientToScreen(window, &point);
		printf("Screen pos %d %d\n", point.x, point.y);
		GetClientRect(window, &dstRect);
		printf("Client rect %d %d %d %d\n", dstRect.left, dstRect.top, dstRect.right, dstRect.bottom);
		renderSize.cx = dstRect.right;
		renderSize.cy = dstRect.bottom;
		OffsetRect(&dstRect, point.x, point.y);
		printf("dst rect %d %d %d %d\n", dstRect.left, dstRect.top, dstRect.right, dstRect.bottom);
	}

	bool exclusive = fullscreen;
	if (!exclusive)
	{
		//r = pdd->SetCooperativeLevel(nullptr, DDSCL_NORMAL);//to use multiple windows
		r = pdd->SetCooperativeLevel(window, DDSCL_NORMAL);
	}
	else
	{
		r = pdd->SetCooperativeLevel(window, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT);
		//r = pdd->SetCooperativeLevel(window, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_ALLOWMODEX);

		int r2 = pdd->SetDisplayMode(renderSize.cx, renderSize.cy, 32);
		printf("%d (%s) IDirectDraw::SetDisplayMode(%d %d %d)\n", r2, ddResultToStr(r2).c_str(), renderSize.cx, renderSize.cy, 32);
	}
	printf("%d (%s) IDirectDraw::SetCooperativeLevel\n", r, ddResultToStr(r).c_str());
	if (r != DD_OK)
		return -1;

	int count = 0;
#if 0
#if 0
	//spams
	count = 0;
	r = pdd->EnumDisplayModes(DDEDM_REFRESHRATES, nullptr, &count, ddEnumModesCallback);
	printf("%d (%s) IDirectDraw::EnumDisplayModes %d\n", r, ddResultToStr(r).c_str(), count);
#endif

	{
		count = 0;
		r = pdd->EnumSurfaces(DDENUMSURFACES_DOESEXIST | DDENUMSURFACES_ALL, nullptr, &count, ddEnumSurfacesCallback);
		printf("%d (%s) IDirectDraw::EnumSurfaces(DOESEXIST | ALL) %d\n", r, ddResultToStr(r).c_str(), count);
		DDSURFACEDESC ddsd{ 0 };
		ddsd.dwSize = sizeof(ddsd);
		//count = 0;
		//r = pdd->EnumSurfaces(DDENUMSURFACES_DOESEXIST, &ddsd, &count, ddEnumSurfacesCallback);
		//printf("%d (%s) IDirectDraw::EnumSurfaces(DOESEXIST) %d\n", r, ddResultToStr(r).c_str(), count);
#if 0
		//spams
		count = 0;
		r = pdd->EnumSurfaces(DDENUMSURFACES_MATCH | DDENUMSURFACES_CANBECREATED, &ddsd, &count, ddEnumSurfacesCallback);
		printf("%d (%s) IDirectDraw::EnumSurfaces(MATCH | CANBECREATED) %d\n", r, ddResultToStr(r).c_str(), count);
#endif
		//count = 0;
		//r = pdd->EnumSurfaces(DDENUMSURFACES_NOMATCH, &ddsd, &count, ddEnumSurfacesCallback);
		//printf("%d (%s) IDirectDraw::EnumSurfaces(NOMATCH) %d\n", r, ddResultToStr(r).c_str(), count);
	}

	DDSURFACEDESC displayModeDesc{};
	displayModeDesc.dwSize = sizeof(displayModeDesc);
	r = pdd->GetDisplayMode(&displayModeDesc);
	printf("%d IDirectDraw::GetDisplayMode\n", r);
	printDDSDesc(displayModeDesc);

	DWORD freq = 0;
	r = pdd->GetMonitorFrequency(&freq);
	printf("%d IDirectDraw::GetMonitorFrequency %d\n", r, freq);

	DWORD numCodes = driverCaps.dwNumFourCCCodes;
	if (numCodes)
	{
		std::vector<DWORD> codes(numCodes);
		r = pdd->GetFourCCCodes(&numCodes, &codes[0]);
		printf("%d IDirectDraw::GetFourCCCodes %d\n", r, numCodes);
		for (int i = 0; i < numCodes; i++)
			printf(" %.4s", &codes[i]);
		printf("\n");
	}

	r = pdd->QueryInterface(IID_IDirectDraw2, (void **)&pdd2);
	printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirectDraw2) %p\n", r, ddResultToStr(r).c_str(), pdd2);
	if (pdd2)
	{
		DDSCAPS ddc{};
		ddc.dwCaps = DDSCAPS_TEXTURE;
		DWORD availMem = 0;
		DWORD freeMem = 0;
		r = pdd2->GetAvailableVidMem(&ddc, &availMem, &freeMem);
		printf("%d IDirectDraw2::GetAvailableVidMem(TEXTURE) %u %u\n", r, availMem, freeMem);
	}
#endif

	r = pdd->QueryInterface(IID_IDirect3D, (void **)&pd3d);
	printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirect3D) %p\n", r, ddResultToStr(r).c_str(), pd3d);

	if (pd3d)
	{
		count = 0;
		pd3d->EnumDevices(d3dEnumDevicesCallback, &count);
		printf("%d (%s) IDirect3D::EnumDevices %d\n", r, ddResultToStr(r).c_str(), count);
		if (r != DD_OK)
			return -1;
	}
	//create surfaces

	DDSURFACEDESC ddsd{ 0 };
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	//DDERR_NOEXCLUSIVEMODE
	bool doublebuffer = exclusive;
	if (doublebuffer)
	{
		ddsd.dwFlags |= DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps |= DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_3DDEVICE;
		ddsd.dwBackBufferCount = 1;
	}

	r = pdd->CreateSurface(&ddsd, &pddsPrimary, nullptr);
	printf("%d (%s) IDirectDraw::CreateSurface(PRIMARY) %p\n", r, ddResultToStr(r).c_str(), pddsPrimary);

	if (doublebuffer)
	{
		if (pddsPrimary)
		{
			DDSCAPS ddsCaps{ DDSCAPS_BACKBUFFER };
			r = pddsPrimary->GetAttachedSurface(&ddsCaps, &pddsBack);
			printf("%d (%s) IDirectDrawSurface::GetAttachedSurface BACK %p\n", r, ddResultToStr(r).c_str(), pddsBack);
		}
	}
	else
	{
		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
		ddsd.dwWidth = renderSize.cx;
		ddsd.dwHeight = renderSize.cy;

		r = pdd->CreateSurface(&ddsd, &pddsBack, nullptr);
		printf("%d (%s) IDirectDraw::CreateSurface(BACK) %p\n", r, ddResultToStr(r).c_str(), pddsBack);

		r = pdd->CreateClipper(0, &pddClipper, nullptr);
		printf("%d (%s) IDirectDraw::CreateClipper %p\n", r, ddResultToStr(r).c_str(), pddClipper);

		if (pddClipper)
		{
			r = pddClipper->SetHWnd(0, window);
			printf("%d (%s) IDirectDrawClipper::SetHWnd\n", r, ddResultToStr(r).c_str());

			if (pddsPrimary)
			{
				r = pddsPrimary->SetClipper(pddClipper);
				printf("%d (%s) IDirectDrawSurface::SetClipper PRIMARY\n", r, ddResultToStr(r).c_str());
			}
		}
	}

#if 0
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;// | DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth = 64;
	ddsd.dwHeight = 64;

	r = pdd->CreateSurface(&ddsd, &pdds1, nullptr);
	printf("%d (%s) IDirectDraw::CreateSurface(OFFSCREEN) %p\n", r, ddResultToStr(r).c_str(), pdds1);
#endif
#if 1
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	ddsd.dwWidth = 64;
	ddsd.dwHeight = 64;
	//mip
	{
	//	ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
	}
	//ddsd.dwWidth = 16;
	//ddsd.dwHeight = 16;
	r = pdd->CreateSurface(&ddsd, &pddsTex, nullptr);
	printf("%d (%s) IDirectDraw::CreateSurface(TEXTURE) %p\n", r, ddResultToStr(r).c_str(), pddsTex);

	count = 0;
	r = pdd->EnumSurfaces(DDENUMSURFACES_DOESEXIST | DDENUMSURFACES_ALL, nullptr, &count, ddEnumSurfacesCallback);
	printf("%d (%s) IDirectDraw::EnumSurfaces(DOESEXIST | ALL) %d\n", r, ddResultToStr(r).c_str(), count);

	{
		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		//ddsd.dwFlags = DDSD_LPSURFACE;
		r = pddsTex->Lock(nullptr, &ddsd, DDLOCK_SURFACEMEMORYPTR, nullptr);
		printf("%d (%s) IDirectDrawSurface::Lock TEXTURE %p\n", r, ddResultToStr(r).c_str(), ddsd.lpSurface);
		if (r == DD_OK)
		{
			printDDSDesc(ddsd);

			int pitch = ddsd.lPitch;
			if (!pitch)
				pitch = ddsd.dwWidth * 4;
			char *dst = (char *)ddsd.lpSurface;
			if (dst)
			{
				for (int i = 0; i < (int)ddsd.dwHeight; i++)
				{
					//memset(dst, 0x80, ddsd.dwWidth * 4);
					for (int j = 0; j < (int)ddsd.dwWidth; j++)
					{
						//bgra
						dst[j * 4 + 0] = 0;
						dst[j * 4 + 1] = (i ^ j * 4) & 0xFF;
						dst[j * 4 + 2] = (j ^ i * 4) & 0xFF;
					}
					dst += pitch;
				}
			}

			r = pddsTex->Unlock(ddsd.lpSurface);
			printf("%d (%s) IDirectDrawSurface::Unlock\n", r, ddResultToStr(r).c_str());
		}
	}

	//RECT srcRect{ 0,0,64,64 };
	//pddsPrimary->Blt(&dstRect, pddsTex, &srcRect, DDBLT_WAIT, nullptr);
#endif

#if 0
	if (pddsBack)
	{
		//POINT offset{ dstRect.left, dstRect.top };
		POINT offset{ 100, 150 };
		for (int yi = 0; yi < 4; yi++)
		{
			for (int xi = 0; xi < 4; xi++)
			{
				RECT dRect{ offset.x + xi * 80, offset.y + yi * 80 };
				dRect.right = dRect.left + 64;
				dRect.bottom = dRect.top + 64;
				DDBLTFX bltFx{ 0 };
				bltFx.dwSize = sizeof(bltFx);
				bltFx.dwFillColor = RGB(50, 180, 255);
				pddsBack->Blt(&dRect, nullptr, nullptr, DDBLT_WAIT | DDBLT_COLORFILL, &bltFx);
			}
		}

		if (pddsPrimary)
		{
			RECT srcRect{ 0,0,renderSize.cx,renderSize.cy };
			//pddsPrimary->BltFast(dstRect.left, dstRect.top, pddsBack, &srcRect, DDBLTFAST_WAIT);
			pddsPrimary->Blt(&dstRect, pddsBack, &srcRect, DDBLT_WAIT, nullptr);
		}
	}
#endif
#if 0
	if (pdds1)
	{
		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_LPSURFACE;
		r = pdds1->Lock(nullptr, &ddsd, DDLOCK_SURFACEMEMORYPTR, nullptr);
		printf("%d (%s) IDirectDrawSurface::Lock %p\n", r, ddResultToStr(r).c_str(), ddsd.lpSurface);
		if (r == DD_OK)
		{
			printDDSDesc(ddsd);

			int pitch = ddsd.lPitch;
			if (!pitch)
				pitch = ddsd.dwWidth * 4;
			char *dst = (char *)ddsd.lpSurface;
			if (dst)
			{
				for (int i = 0; i < (int)ddsd.dwHeight; i++)
				{
					//memset(dst, 0x80, ddsd.dwWidth * 4);
					for (int j = 0; j < (int)ddsd.dwWidth; j++)
					{
						//bgra
						dst[j * 4 + 0] = 0;
						dst[j * 4 + 1] = (i * 4) & 0xFF;
						dst[j * 4 + 2] = (j * 4) & 0xFF;
					}
					dst += pitch;
				}
			}

			r = pdds1->Unlock(ddsd.lpSurface);
			printf("%d (%s) IDirectDrawSurface::Unlock\n", r, ddResultToStr(r).c_str());

			if (pddsPrimary)
			{
				for (int yi = 0; yi < 4; yi++)
				{
					for (int xi = 0; xi < 4; xi++)
					{
						RECT dstRect{ 100 + xi * 80, 150 + yi * 80 };
						dstRect.right = dstRect.left + 64;
						dstRect.bottom = dstRect.top + 64;
						RECT srcRect{ 0,0,64,64 };
						//DDBLTFX bltFx{ 0 };
						//bltFx.dwSize = sizeof(bltFx);
						//pddsPrimary->Blt(&dstRect, pdds1, &srcRect, DDBLT_WAIT, nullptr);// &bltFx);

						pddsPrimary->BltFast(dstRect.left, dstRect.top, pdds1, &srcRect, DDBLTFAST_WAIT);
					}
				}
			}
		}
	}
#endif

#if 0
	{
		IDirectDrawSurface *pddsGDI = nullptr;
		r = pdd->GetGDISurface(&pddsGDI);
		printf("%d (%s) IDirectDraw::GetGDISurface %p\n", r, ddResultToStr(r).c_str(), pddsGDI);
		VerboseRelease(pddsGDI, "IDirectDrawSurface::Release GDI");
	}
#endif

	return 0;
}

// one of D3DTRANSFORMSTATETYPE for D3DOP_STATETRANSFORM
// one of D3DLIGHTSTATETYPE for D3DOP_STATELIGHT
// one of D3DRENDERSTATETYPE for D3DOP_STATERENDER
HRESULT TestContext::d3dSetState(D3DOPCODE op, int state_type, DWORD value)
{
	printf("d3dSetState(%d %d %d)\n", op, state_type, value);
	int size = sizeof(D3DINSTRUCTION) * 2;
	size += sizeof(D3DSTATE) * 1;
	//printf("size %d\n", size);

	D3DEXECUTEBUFFERDESC ebd{};
	ebd.dwSize = sizeof(ebd);
	ebd.dwFlags = D3DDEB_BUFSIZE;
	ebd.dwBufferSize = size;
	IDirect3DExecuteBuffer *peb = nullptr;
	HRESULT r = pd3dd->CreateExecuteBuffer(&ebd, &peb, nullptr);
	printf("%d (%s) IDirect3DDevice::CreateExecuteBuffer %p\n", r, ddResultToStr(r).c_str(), peb);

	{
		D3DEXECUTEBUFFERDESC ebDesc{};
		ebDesc.dwSize = sizeof(ebDesc);
		ebDesc.dwFlags = D3DDEB_LPDATA;
		r = peb->Lock(&ebDesc);
		printf("%d (%s) IDirect3DExecuteBuffer::Lock %p\n", r, ddResultToStr(r).c_str(), ebDesc.lpData);

		void *cur = ebDesc.lpData;

		PUTD3DINSTRUCTION(op, sizeof(D3DSTATE), 1, cur);
		STATE_DATA(state_type, value, cur);
		OP_EXIT(cur);

		D3DEXECUTEDATA exData{};
		exData.dwSize = sizeof(exData);
		exData.dwInstructionLength = ((char *)cur - (char *)ebDesc.lpData);

		r = peb->Unlock();
		printf("%d (%s) IDirect3DExecuteBuffer::Unlock %d/%d\n", r, ddResultToStr(r).c_str(), int((char *)cur - (char *)ebDesc.lpData), ebDesc.dwBufferSize);

		r = peb->SetExecuteData(&exData);
		printf("%d (%s) IDirect3DExecuteBuffer::SetExecuteData\n", r, ddResultToStr(r).c_str());
	}

	r = pd3dd->BeginScene();
	printf("%d (%s) IDirect3DDevice::BeginScene\n", r, ddResultToStr(r).c_str());

	r = pd3dd->Execute(peb, pd3dViewport, D3DEXECUTE_UNCLIPPED);
	printf("%d (%s) IDirect3DDevice::Execute\n", r, ddResultToStr(r).c_str());

	r = pd3dd->EndScene();
	printf("%d (%s) IDirect3DDevice::EndScene\n", r, ddResultToStr(r).c_str());

	VerboseRelease(peb, "IDirect3DExecuteBuffer::Release");
	return r;
}

int testTLTriangle(const D3DEXECUTEBUFFERDESC &ebDesc, D3DEXECUTEDATA &exData)
{
	const int vertsNum = 7;

	D3DTLVERTEX verts[vertsNum]{};
	memset(verts, 0, sizeof(verts));
	verts[0].color = RGBA_MAKE(250, 10, 10, 255);
	verts[0].sx = 10;
	verts[0].sy = 400;
	verts[0].rhw = 1;
	verts[1].color = RGBA_MAKE(10, 250, 10, 255);
	verts[1].sx = 300;
	verts[1].sy = 10;
	verts[1].rhw = 1;
	verts[2].color = RGBA_MAKE(10, 10, 250, 255);
	verts[2].sx = 600;
	verts[2].sy = 400;
	verts[2].rhw = 1;

	verts[3].color = RGBA_MAKE(250, 250, 10, 255);
	verts[3].sx = 30;
	verts[3].sy = 20;
	verts[3].rhw = 1;

	memcpy(&verts[4], verts, sizeof(D3DTLVERTEX) * 3);
	verts[4].sx = 10;
	verts[4].sy = 30;
	verts[5].sx = 30;
	verts[5].sy = 10;
	verts[6].sx = 60;
	verts[6].sy = 40;

	D3DTRIANGLE tri{};
	tri.v1 = 0;
	tri.v2 = 1;
	tri.v3 = 2;
	tri.wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;

	D3DLINE lines[3]{};
	lines[0].v1 = 4;
	lines[0].v2 = 5;
	lines[1].v1 = 5;
	lines[1].v2 = 6;
	lines[2].v1 = 6;
	lines[2].v2 = 4;

	void *cur = ebDesc.lpData;

	VERTEX_DATA(verts, vertsNum, cur);
	void *insStart = cur;
	OP_PROCESS_VERTICES(1, cur);
	PROCESSVERTICES_DATA(D3DPROCESSVERTICES_COPY | D3DPROCESSVERTICES_UPDATEEXTENTS, 0, vertsNum, cur);
	if (QWORD_ALIGNED(cur)) {
		OP_NOP(cur);
	}

	OP_POINT_LIST(3, 1, cur);

	OP_TRIANGLE_LIST(1, cur);
	TRIANGLE_LIST_DATA(&tri, 1, cur);

	if (QWORD_ALIGNED(cur)) {
		OP_NOP(cur);
	}

	OP_LINE_LIST(3, cur);
	LINE_LIST_DATA(lines, 3, cur);

	OP_EXIT(cur);

	exData.dwVertexCount = vertsNum;
	exData.dwVertexOffset = 0;
	exData.dwInstructionOffset = ((char *)insStart - (char *)ebDesc.lpData);
	exData.dwInstructionLength = ((char *)cur - (char *)insStart);

	return (char *)cur - (char *)ebDesc.lpData;
}

int testLTriangle(D3DEXECUTEBUFFERDESC &ebDesc, D3DEXECUTEDATA &exData)
{
	const int vertsNum = 7;

	D3DLVERTEX verts[vertsNum]{};
	memset(verts, 0, sizeof(verts));
	verts[0].color = RGBA_MAKE(250, 10, 10, 255);
	verts[0].x = -0.9f;
	verts[0].y = -0.9f;
	verts[1].color = RGBA_MAKE(10, 250, 10, 255);
	verts[1].x = 0;
	verts[1].y = 0.9f;
	verts[2].color = RGBA_MAKE(10, 10, 250, 255);
	verts[2].x = 0.9f;
	verts[2].y = -0.9f;

	verts[3].color = RGBA_MAKE(250, 250, 10, 255);
	verts[3].x = -0.9f;
	verts[3].y = 0.91f;

	memcpy(&verts[4], verts, sizeof(D3DTLVERTEX) * 3);
	verts[4].x = -0.95f;
	verts[4].y = 0.9f;
	verts[5].x = -0.9f;
	verts[5].y = 0.95f;
	verts[6].x = -0.85f;
	verts[6].y = 0.88f;

	D3DTRIANGLE tri{};
	tri.v1 = 0;
	tri.v2 = 1;
	tri.v3 = 2;
	tri.wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;

	D3DLINE lines[3]{};
	lines[0].v1 = 4;
	lines[0].v2 = 5;
	lines[1].v1 = 5;
	lines[1].v2 = 6;
	lines[2].v1 = 6;
	lines[2].v2 = 4;

	void *cur = ebDesc.lpData;

	VERTEX_DATA(verts, vertsNum, cur);
	void *insStart = cur;
	OP_PROCESS_VERTICES(1, cur);
	PROCESSVERTICES_DATA(D3DPROCESSVERTICES_TRANSFORM | D3DPROCESSVERTICES_UPDATEEXTENTS, 0, vertsNum, cur);
	if (QWORD_ALIGNED(cur)) {
		OP_NOP(cur);
	}

	//bug: acts as if wFirst always 1, when D3DPROCESSVERTICES_TRANSFORM
	OP_POINT_LIST(3, 1, cur);

	OP_TRIANGLE_LIST(1, cur);
	TRIANGLE_LIST_DATA(&tri, 1, cur);

	if (QWORD_ALIGNED(cur)) {
		OP_NOP(cur);
	}

	OP_LINE_LIST(3, cur);
	LINE_LIST_DATA(lines, 3, cur);

	OP_EXIT(cur);

	exData.dwVertexCount = vertsNum;
	exData.dwVertexOffset = 0;
	exData.dwInstructionOffset = ((char *)insStart - (char *)ebDesc.lpData);
	exData.dwInstructionLength = ((char *)cur - (char *)insStart);

	return (char *)cur - (char *)ebDesc.lpData;
}

int testTexturedTriangle(D3DEXECUTEBUFFERDESC &ebDesc, D3DEXECUTEDATA &exData, D3DTEXTUREHANDLE texHandle)
{
	const int vertsNum = 3;

	D3DLVERTEX verts[vertsNum]{};
	memset(verts, 0, sizeof(verts));
	verts[0].color = RGBA_MAKE(250, 10, 10, 255);
	verts[0].x = -0.9f;
	verts[0].y = -0.9f;
	verts[0].dvTU = 0;
	verts[0].dvTV = 0;
	verts[1].color = RGBA_MAKE(10, 250, 10, 255);
	verts[1].x = 0;
	verts[1].y = 0.9f;
	verts[1].dvTU = 0.5f;
	verts[1].dvTV = 1;
	verts[2].color = RGBA_MAKE(10, 10, 250, 255);
	verts[2].x = 0.9f;
	verts[2].y = -0.9f;
	verts[2].dvTU = 1;
	verts[2].dvTV = 0;

	D3DTRIANGLE tri{};
	tri.v1 = 0;
	tri.v2 = 1;
	tri.v3 = 2;
	tri.wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;

	void *cur = ebDesc.lpData;

	VERTEX_DATA(verts, vertsNum, cur);
	void *insStart = cur;
	OP_PROCESS_VERTICES(1, cur);
	PROCESSVERTICES_DATA(D3DPROCESSVERTICES_TRANSFORM | D3DPROCESSVERTICES_UPDATEEXTENTS, 0, vertsNum, cur);
	if (QWORD_ALIGNED(cur)) {
		OP_NOP(cur);
	}

	OP_STATE_RENDER(2, cur);
	STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, texHandle, cur);
	STATE_DATA(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_DECAL, cur);

	OP_TRIANGLE_LIST(1, cur);
	TRIANGLE_LIST_DATA(&tri, 1, cur);

	OP_EXIT(cur);

	exData.dwVertexCount = vertsNum;
	exData.dwVertexOffset = 0;
	exData.dwInstructionOffset = ((char *)insStart - (char *)ebDesc.lpData);
	exData.dwInstructionLength = ((char *)cur - (char *)insStart);

	return (char *)cur - (char *)ebDesc.lpData;
}

int TestContext::initD3D()
{
	HRESULT r;

	if (!pd3d)
		return -1;

	D3DFINDDEVICESEARCH search{};
	search.dwSize = sizeof(search);
	search.dwFlags = D3DFDS_COLORMODEL;
	search.dcmColorModel = D3DCOLOR_RGB;
#if 0
	{
		search.dwFlags |= D3DFDS_HARDWARE;
		search.bHardware = false;
	}
#endif
	D3DFINDDEVICERESULT result{};
	result.dwSize = sizeof(result);

	r = pd3d->FindDevice(&search, &result);
	printf("%d (%s) IDirect3D::FindDevice %X\n", r, ddResultToStr(r).c_str(), result.guid.Data1);

	IDirectDrawSurface *surf = pddsBack;

	if (pddsBack)
	{
		r = pddsBack->QueryInterface(result.guid, (void **)&pd3dd);
		printf("%d (%s) IDirectDrawSurface::QueryInterface(IID_IDirect3DDevice) BACK %p\n", r, ddResultToStr(r).c_str(), pd3dd);
	}

#if 0
	if (!pd3dd && pdds1)
	{
		surf = pdds1;
		r = pdds1->QueryInterface(IID_IDirect3DDevice, (void **)&pd3dd);
		printf("%d (%s) IDirectDrawSurface::QueryInterface(IID_IDirect3DDevice) OFFSCREEN %p\n", r, ddResultToStr(r).c_str(), pd3dd);
	}
#endif
	if (!pd3dd)
		return -1;

	int count = 0;
	r = pd3dd->EnumTextureFormats(d3dEnumTexFmtCallback, &count);
	printf("%d (%s) IDirect3DDevice::EnumTextureFormats %d\n", r, ddResultToStr(r).c_str(), count);

	D3DDEVICEDESC hwdesc{ sizeof(D3DDEVICEDESC) };
	D3DDEVICEDESC heldesc{ sizeof(D3DDEVICEDESC) };
	r = pd3dd->GetCaps(&hwdesc, &heldesc);
	printf("%d (%s) IDirect3DDevice::GetCaps\n", r, ddResultToStr(r).c_str());
	printf(" dwMaxBufferSize %d %d dwMaxVertexCount %d %d\n", hwdesc.dwMaxBufferSize, heldesc.dwMaxBufferSize, hwdesc.dwMaxVertexCount, heldesc.dwMaxVertexCount);

#if 1
	if (pddsTex)
	{
		r = pddsTex->QueryInterface(IID_IDirect3DTexture, (void **)&pd3dTex);
		printf("%d (%s) IDirectDrawSurface::QueryInterface(IID_IDirect3DTexture) %p\n", r, ddResultToStr(r).c_str(), pd3dTex);

		int count = 0;
		r = pddsTex->EnumAttachedSurfaces(&count, ddEnumSurfacesCallback);
		printf("%d (%s) IDirectDrawSurface::EnumAttachedSurfaces TEX %d\n", r, ddResultToStr(r).c_str(), count);

		r = pd3dTex->GetHandle(pd3dd, &texHandle);
		printf("%d (%s) IDirect3DTexture::GetHandle %d\n", r, ddResultToStr(r).c_str(), texHandle);

		//r = pd3dTex->Load(pd3dTex);
		//printf("%d (%s) IDirect3DTexture::Load\n", r, ddResultToStr(r).c_str());
	}
#endif

	r = pd3d->CreateViewport(&pd3dViewport, nullptr);
	printf("%d (%s) IDirect3D::CreateViewport %p\n", r, ddResultToStr(r).c_str(), pd3dViewport);

	if (!pd3dViewport)
		return -1;

	r = pd3dd->AddViewport(pd3dViewport);
	printf("%d (%s) IDirect3DDevice::AddViewport\n", r, ddResultToStr(r).c_str());

	DDSURFACEDESC desc{};
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
	surf->GetSurfaceDesc(&desc);

	D3DVIEWPORT viewport{};
	viewport.dwSize = sizeof(viewport);
	viewport.dwX = 0;
	viewport.dwY = 0;
	viewport.dwWidth = desc.dwWidth;
	viewport.dwHeight = desc.dwHeight;
	viewport.dvScaleX = 0.5f * desc.dwWidth;
	viewport.dvScaleY = 0.5f * desc.dwHeight;
	viewport.dvMaxX = 1;
	viewport.dvMaxY = 1;
	//viewport.dvMinZ = -1;
	//viewport.dvMaxZ = 1;
	r = pd3dViewport->SetViewport(&viewport);
	printf("%d (%s) IDirect3DViewport::SetViewport\n", r, ddResultToStr(r).c_str());

	r = pd3dd->CreateMatrix(&mthWorld);
	printf("%d (%s) IDirect3DDevice::CreateMatrix %d\n", r, ddResultToStr(r).c_str(), mthWorld);

	d3dSetState(D3DOP_STATETRANSFORM, D3DTRANSFORMSTATE_WORLD, mthWorld);

#if 0
	int size = sizeof(D3DVERTEX) * 3;
	size += sizeof(D3DINSTRUCTION) * 6;
	size += sizeof(D3DSTATE) * 2;
	size += sizeof(D3DPROCESSVERTICES);
	size += sizeof(D3DTRIANGLE) * 1;
	printf("size %d\n", size);
#endif

	D3DEXECUTEBUFFERDESC ebd{};
	ebd.dwSize = sizeof(ebd);
	ebd.dwFlags = D3DDEB_BUFSIZE;
	ebd.dwBufferSize = 1024;
	r = pd3dd->CreateExecuteBuffer(&ebd, &pd3deb, nullptr);
	printf("%d (%s) IDirect3DDevice::CreateExecuteBuffer %p\n", r, ddResultToStr(r).c_str(), pd3deb);

	{
		D3DEXECUTEBUFFERDESC ebDesc{};
		ebDesc.dwSize = sizeof(ebDesc);
		ebDesc.dwFlags = D3DDEB_LPDATA;
		r = pd3deb->Lock(&ebDesc);
		printf("%d (%s) IDirect3DExecuteBuffer::Lock %p\n", r, ddResultToStr(r).c_str(), ebDesc.lpData);

		D3DEXECUTEDATA exData{};
		exData.dwSize = sizeof(exData);
		//int l = testTLTriangle(ebDesc, exData);
		//int l = testLTriangle(ebDesc, exData);
		int l = testTexturedTriangle(ebDesc, exData, texHandle);

		r = pd3deb->Unlock();
		printf("%d (%s) IDirect3DExecuteBuffer::Unlock %d/%d\n", r, ddResultToStr(r).c_str(), l, ebDesc.dwBufferSize);

		r = pd3deb->SetExecuteData(&exData);
		printf("%d (%s) IDirect3DExecuteBuffer::SetExecuteData\n", r, ddResultToStr(r).c_str());
	}

	r = pd3dd->BeginScene();
	printf("%d (%s) IDirect3DDevice::BeginScene\n", r, ddResultToStr(r).c_str());

	r = pd3dd->Execute(pd3deb, pd3dViewport, D3DEXECUTE_CLIPPED);// D3DEXECUTE_UNCLIPPED);
	printf("%d (%s) IDirect3DDevice::Execute\n", r, ddResultToStr(r).c_str());

	r = pd3dd->EndScene();
	printf("%d (%s) IDirect3DDevice::EndScene\n", r, ddResultToStr(r).c_str());

	D3DSTATS d3dStats{};
	d3dStats.dwSize = sizeof(d3dStats);
	r = pd3dd->GetStats(&d3dStats);
	printf("%d (%s) IDirect3DDevice::GetStats\n", r, ddResultToStr(r).c_str());
	printf(" dwTrianglesDrawn %d dwLinesDrawn %d dwPointsDrawn %d dwSpansDrawn %d dwVerticesProcessed %d\n",
		d3dStats.dwTrianglesDrawn,
		d3dStats.dwLinesDrawn,
		d3dStats.dwPointsDrawn,
		d3dStats.dwSpansDrawn,
		d3dStats.dwVerticesProcessed);

	return 0;
}

void TestContext::draw()
{
	if (!pddsPrimary || !pddsBack)
		return;

	static int frame = 0;

	//
	float ct = cos(frame * 0.01f);
	float st = sin(frame * 0.01f);
	D3DMATRIX mtxWorld = {
	ct,st,0,0,
	-st,ct,0,0,
	0,0,1,0,
	0,0,0,1
	};
	HRESULT r = pd3dd->SetMatrix(mthWorld, &mtxWorld);
	if (r) printf("%d (%s) IDirect3DDevice::SetMatrix\n", r, ddResultToStr(r).c_str());

	r = pd3dd->BeginScene();
	if (r) printf("%d (%s) IDirect3DDevice::BeginScene\n", r, ddResultToStr(r).c_str());
	r = pd3dd->Execute(pd3deb, pd3dViewport, D3DEXECUTE_CLIPPED);
	if (r) printf("%d (%s) IDirect3DDevice::Execute\n", r, ddResultToStr(r).c_str());
	r = pd3dd->EndScene();
	if (r) printf("%d (%s) IDirect3DDevice::EndScene\n", r, ddResultToStr(r).c_str());
	//

	SIZE renderSize{};
	DDSURFACEDESC desc{};
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
	pddsBack->GetSurfaceDesc(&desc);
	renderSize.cx = desc.dwWidth;
	renderSize.cy = desc.dwHeight;

	RECT dstRect{};
	POINT point{ 0,0 };
	ClientToScreen(window, &point);
	GetClientRect(window, &dstRect);
	//renderSize.cx = dstRect.right;
	//renderSize.cy = dstRect.bottom;
	OffsetRect(&dstRect, point.x, point.y);

#if 0
	//
	RECT dRect{ (frame * 65) % renderSize.cx, 50 };
	dRect.right = dRect.left + 64;
	dRect.bottom = dRect.top + 64;
	DDBLTFX bltFx{ 0 };
	bltFx.dwSize = sizeof(bltFx);
	bltFx.dwFillColor = RGB((frame * 15) % 255, (frame * 27) % 255, 110);
	pddsBack->Blt(&dRect, nullptr, nullptr, DDBLT_WAIT | DDBLT_COLORFILL, &bltFx);
	//
#endif
	frame++;

	RECT srcRect{ 0,0,renderSize.cx,renderSize.cy };
	pddsPrimary->Blt(&dstRect, pddsBack, &srcRect, DDBLT_WAIT, NULL);
}

void TestContext::shutdownD3D()
{
	HRESULT r = pd3dd->DeleteMatrix(mthWorld);
	printf("%d (%s) IDirect3DDevice::DeleteMatrix %d\n", r, ddResultToStr(r).c_str(), mthWorld);
	mthWorld = 0;

	r = pd3dd->DeleteViewport(pd3dViewport);
	printf("%d (%s) IDirect3DDevice::DeleteViewport\n", r, ddResultToStr(r).c_str());

	VerboseRelease(pd3dViewport, "IDirect3DViewport::Release");
	VerboseRelease(pd3deb, "IDirect3DExecuteBuffer::Release");
}

void TestContext::shutdownDD()
{
	VerboseRelease(pd3dTex, "IDirect3DTexture::Release");
	VerboseRelease(pddsTex, "IDirectDrawSurface::Release TEXTURE");

	VerboseRelease(pdds1, "IDirectDrawSurface::Release OFFSCREEN");

	VerboseRelease(pd3dd, "IDirect3DDevice::Release");
	VerboseRelease(pddsBack, "IDirectDrawSurface::Release BACK");

	VerboseRelease(pddsPrimary, "IDirectDrawSurface::Release PRIMARY");

	VerboseRelease(pddClipper, "IDirectDrawClipper::Release");

	VerboseRelease(pd3d, "IDirect3D::Release");
	VerboseRelease(pdd2, "IDirectDraw2::Release");
	VerboseRelease(pdd, "IDirectDraw::Release");
}

int main()
{
	SetConsoleOutputCP(1251);
	//SetConsoleOutputCP(CP_UTF8);

	//printf("Hello World!\n");

	//if (FAILED(CoInitialize(NULL)))
	//	return -1;

	TestContext context;
	if (context.initWin(640, 480, false))
		return -1;
	if (context.initDD())
		return -1;
	if (context.initD3D())
		return -1;
	context.loop();
	context.shutdownWin();

	return 0;
}
