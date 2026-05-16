
#include <stdio.h>
#include <string>

#define INITGUID
#include <d3d.h>

#pragma comment(lib, "ddraw.lib")

struct TestContext
{
	LPCTSTR class_name = TEXT("DXTestWin");
	HWND window;
	bool fullscreen = false;
	bool shouldClose = false;
	IDirectDraw7* pdd = nullptr;
	IDirect3D7* pd3d = nullptr;
	IDirectDrawSurface7* pddsPrimary = nullptr;
	IDirectDrawSurface7* pddsBack = nullptr;
	IDirectDrawClipper* pddClipper = nullptr;
	IDirect3DDevice7* pd3dd = nullptr;

	bool initWin(int w, int h, bool fullscreen);
	int initDD();
	int initD3D();
	void loop();
	void draw();
	void onWinDestroy();
	void shutdownWin();
	void shutdownDD();
};

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TestContext* ctx = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		ctx = (TestContext*)((CREATESTRUCT*)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)ctx);
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	ctx = (TestContext*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (uMsg) {
	case WM_DESTROY:
		if (ctx)
			ctx->onWinDestroy();
		PostQuitMessage(0);
		return 0;

	case WM_TIMER:
	{
		if (ctx)
			ctx->draw();
		break;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		HDC dc = BeginPaint(hWnd, &paint);
		if (!dc)
			return 0;

		BOOL r = EndPaint(hWnd, &paint);
		return 0;
	}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool TestContext::initWin(int w, int h, bool fs)
{
	fullscreen = fs;
	printf("initWin(%d,%d,%s)\n", w, h, fs ? "fullscreen" : "windowed");

	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	if (fullscreen)
		style = WS_POPUP | WS_VISIBLE;
	DWORD exStyle = WS_EX_APPWINDOW;
	RECT winSize{ 0,0,w,h };
	BOOL r = AdjustWindowRectEx(&winSize, style, false, exStyle);
	printf("%d AdjustWindowRectEx %d %d %d %d\n", r, winSize.left, winSize.top, winSize.right, winSize.bottom);

	HINSTANCE inst = GetModuleHandle(nullptr);
	WNDCLASS wndClass{};
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WindowProc;
	wndClass.hInstance = inst;
	wndClass.lpszClassName = class_name;
	ATOM wc = RegisterClass(&wndClass);
	printf("RegisterClass = %X\n", wc);
	if (!wc)
		return true;
	window = CreateWindowEx(exStyle, class_name, TEXT("DX7 test"), style,
		CW_USEDEFAULT, CW_USEDEFAULT, winSize.right - winSize.left, winSize.bottom - winSize.top, nullptr, nullptr, inst, this);
	printf("CreateWindowEx = %p\n", window);
	if (!window)
		return true;
	r = UpdateWindow(window);
	printf("UpdateWindow = %d\n", r);

	return false;
}

void TestContext::onWinDestroy()
{
	window = nullptr;
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

void TestContext::loop()
{
	if (!SetTimer(window, 1, 500, NULL))
		return;

	while (!shouldClose)
	{
		MSG msg{};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (GetAsyncKeyState(VK_ESCAPE))
			shouldClose = true;
	}
}

std::string ddResultToStr(HRESULT r)
{
	if (r == DD_OK)
		return "DD_OK";

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

	if (lpMsgBuf)
	{
		std::string ret(lpMsgBuf);
		LocalFree(lpMsgBuf);
		return ret;
	}

	int sev = (r >> 31) & 1;
	int fac = (r >> 16) & 0x7FFF;
	int code = r & 0xFFFF;
	char buff[256]{};
	snprintf(buff, 255, "%d 0x%X %d", sev, fac, code);
	return std::string(buff);
}

int TestContext::initDD()
{
	HRESULT r;
	if (!pdd)
	{
		r = DirectDrawCreateEx(nullptr, (LPVOID*)&pdd, IID_IDirectDraw7, nullptr);
		printf("%d (%s) DirectDrawCreateEx %p\n", r, ddResultToStr(r).c_str(), pdd);
		if (!pdd || r != DD_OK)
		{
			printf("DX7 not supported\n");
			return -1;
		}
	}

	SIZE renderSize{};
	if (window)
	{
		RECT dstRect{};
		GetClientRect(window, &dstRect);
		printf("GetClientRect %d %d %d %d\n", dstRect.left, dstRect.top, dstRect.right, dstRect.bottom);
		renderSize.cx = dstRect.right;
		renderSize.cy = dstRect.bottom;
	}

	bool exclusive = fullscreen;
	if (!exclusive)
	{
		r = pdd->SetCooperativeLevel(window, DDSCL_NORMAL);
	}
	else
	{
		r = pdd->SetCooperativeLevel(window, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT);
	}
	printf("%d (%s) IDirectDraw7::SetCooperativeLevel\n", r, ddResultToStr(r).c_str());
	if (r != DD_OK)
		return -1;

	if (exclusive)
	{
		r = pdd->SetDisplayMode(renderSize.cx, renderSize.cy, 32, 0, 0);
		printf("%d (%s) IDirectDraw7::SetDisplayMode(%d %d %d)\n", r, ddResultToStr(r).c_str(), renderSize.cx, renderSize.cy, 32);
	}

	int count = 0;

	r = pdd->QueryInterface(IID_IDirect3D7, (void**)&pd3d);
	printf("%d (%s) IDirectDraw7::QueryInterface(IID_IDirect3D7) %p\n", r, ddResultToStr(r).c_str(), pd3d);

	//create surfaces
	DDSURFACEDESC2 ddsd{ 0 };
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	bool doublebuffer = exclusive;
	if (doublebuffer)
	{
		ddsd.dwFlags |= DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps |= DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_3DDEVICE;
		ddsd.dwBackBufferCount = 1;
	}

	r = pdd->CreateSurface(&ddsd, &pddsPrimary, nullptr);
	printf("%d (%s) IDirectDraw7::CreateSurface(PRIMARY) %p\n", r, ddResultToStr(r).c_str(), pddsPrimary);

	if (doublebuffer)
	{
		if (pddsPrimary)
		{
			DDSCAPS2 ddsCaps{ DDSCAPS_BACKBUFFER };
			r = pddsPrimary->GetAttachedSurface(&ddsCaps, &pddsBack);
			printf("%d (%s) IDirectDrawSurface7::GetAttachedSurface BACK %p\n", r, ddResultToStr(r).c_str(), pddsBack);
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
		//half res
		//ddsd.dwWidth >>= 1;
		//ddsd.dwHeight >>= 1;
		//

		r = pdd->CreateSurface(&ddsd, &pddsBack, nullptr);
		printf("%d (%s) IDirectDraw7::CreateSurface(BACK) %p\n", r, ddResultToStr(r).c_str(), pddsBack);

		r = pdd->CreateClipper(0, &pddClipper, nullptr);
		printf("%d (%s) IDirectDraw7::CreateClipper %p\n", r, ddResultToStr(r).c_str(), pddClipper);

		if (pddClipper)
		{
			r = pddClipper->SetHWnd(0, window);
			printf("%d (%s) IDirectDrawClipper::SetHWnd\n", r, ddResultToStr(r).c_str());

			if (pddsPrimary)
			{
				r = pddsPrimary->SetClipper(pddClipper);
				printf("%d (%s) IDirectDrawSurface7::SetClipper PRIMARY\n", r, ddResultToStr(r).c_str());
			}
		}
	}

	return 0;
}

template<typename T>
int VerboseRelease(T*& obj, const char* msg)
{
	if (!obj)
		return 0;
	int c = obj->Release();
	obj = nullptr;
	printf("%d %s\n", c, msg);
	return c;
}

int TestContext::initD3D()
{
	HRESULT r;

	if (!pd3d || !pddsBack)
		return -1;

	r = pd3d->CreateDevice(IID_IDirect3DTnLHalDevice, pddsBack, &pd3dd);
	printf("%d (%s) IDirect3D7::CreateDevice(IID_IDirect3DTnLHalDevice) BACK %p\n", r, ddResultToStr(r).c_str(), pd3dd);

	if (!pd3dd)
		return -1;

	D3DDEVICEDESC7 hwCaps{ sizeof(D3DDEVICEDESC7) };
	r = pd3dd->GetCaps(&hwCaps);
	printf("%d (%s) IDirect3DDevice7::GetCaps\n", r, ddResultToStr(r).c_str());

	DDSURFACEDESC2 desc{};
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
	pddsBack->GetSurfaceDesc(&desc);

	r = pd3dd->Clear(0, nullptr, D3DCLEAR_TARGET, RGBA_MAKE(20, 30, 40, 0), 0, 0);
	printf("%d (%s) IDirect3DDevice7::Clear2(D3DCLEAR_TARGET)\n", r, ddResultToStr(r).c_str());

	struct vertex_t {
		D3DVALUE    sx;
		D3DVALUE    sy;
		D3DVALUE    sz;
		D3DVALUE    rhw;
		D3DCOLOR    color;
	} verts[3]{};
	verts[0].sx = 10;
	verts[0].sy = desc.dwHeight * 0.9f;
	verts[0].sz = 0.5;
	verts[0].rhw = 1;
	verts[0].color = RGBA_MAKE(250, 10, 10, 255);
	verts[1].sx = desc.dwWidth * 0.5f;
	verts[1].sy = 10;
	verts[1].sz = 0.5;
	verts[1].rhw = 1;
	verts[1].color = RGBA_MAKE(10, 250, 10, 255);
	verts[2].sx = desc.dwWidth * 0.9f;
	verts[2].sy = desc.dwHeight * 0.8f;
	verts[2].sz = 0.5;
	verts[2].rhw = 1;
	verts[2].color = RGBA_MAKE(10, 10, 250, 255);

	r = pd3dd->BeginScene();
	printf("%d (%s) IDirect3DDevice7::BeginScene\n", r, ddResultToStr(r).c_str());

	D3DVIEWPORT7 viewport{};
	viewport.dwX = 0;
	viewport.dwY = 0;
	viewport.dwWidth = desc.dwWidth;
	viewport.dwHeight = desc.dwHeight;
	viewport.dvMinZ = 0;
	viewport.dvMaxZ = 1;
	r = pd3dd->SetViewport(&viewport);
	printf("%d (%s) IDirect3DDevice7::SetViewport\n", r, ddResultToStr(r).c_str());

	if (hwCaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)
	{
		r = pd3dd->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTINDEPENDENT);
		printf("%d (%s) IDirect3DDevice7::SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTINDEPENDENT)\n", r, ddResultToStr(r).c_str());
	}

	r = pd3dd->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, verts, 3, 0);
	printf("%d (%s) IDirect3DDevice7::DrawPrimitive\n", r, ddResultToStr(r).c_str());

	r = pd3dd->EndScene();
	printf("%d (%s) IDirect3DDevice7::EndScene\n", r, ddResultToStr(r).c_str());

	return 0;
}

void TestContext::draw()
{
	if (!pddsPrimary || !pddsBack)
		return;

	DDSURFACEDESC2 desc{};
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
	pddsBack->GetSurfaceDesc(&desc);
	RECT srcRect{ 0, 0, (LONG)desc.dwWidth, (LONG)desc.dwHeight };

	RECT dstRect{};
	POINT point{ 0, 0 };
	ClientToScreen(window, &point);
	GetClientRect(window, &dstRect);
	OffsetRect(&dstRect, point.x, point.y);

	pddsPrimary->Blt(&dstRect, pddsBack, &srcRect, DDBLT_WAIT, NULL);
}

void TestContext::shutdownDD()
{
	VerboseRelease(pd3dd, "IDirect3DDevice7::Release");
	VerboseRelease(pddsBack, "IDirectDrawSurface7::Release BACK");
	VerboseRelease(pddsPrimary, "IDirectDrawSurface7::Release PRIMARY");
	VerboseRelease(pddClipper, "IDirectDrawClipper::Release");
	VerboseRelease(pd3d, "IDirect3D7::Release");
	VerboseRelease(pdd, "IDirectDraw7::Release");}

int main()
{
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
