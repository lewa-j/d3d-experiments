
#include <stdio.h>
#include <string>

#define INITGUID
#define DIRECT3D_VERSION 0x0500
#include <d3d.h>

#pragma comment(lib, "ddraw.lib")

struct TestContext
{
	LPCTSTR class_name = TEXT("DXTestWin");
	HWND window;
	bool fullscreen = false;
	bool shouldClose = false;
	IDirectDraw* pdd = nullptr;
	IDirect3D2* pd3d2 = nullptr;
	IDirectDrawSurface* pddsPrimary = nullptr;
	IDirectDrawSurface* pddsBack = nullptr;
	IDirectDrawClipper* pddClipper = nullptr;
	IDirect3DDevice2* pd3dd2 = nullptr;
	IDirect3DViewport2* pd3dViewport2 = nullptr;

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
	window = CreateWindowEx(exStyle, class_name, TEXT("DX5 test"), style,
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
		r = DirectDrawCreate(nullptr, &pdd, nullptr);
		printf("%d (%s) DirectDrawCreate %p\n", r, ddResultToStr(r).c_str(), pdd);
		if (r != DD_OK)
			return -1;
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
	printf("%d (%s) IDirectDraw::SetCooperativeLevel\n", r, ddResultToStr(r).c_str());
	if (r != DD_OK)
		return -1;

	if (exclusive)
	{
		r = pdd->SetDisplayMode(renderSize.cx, renderSize.cy, 32);
		printf("%d (%s) IDirectDraw::SetDisplayMode(%d %d %d)\n", r, ddResultToStr(r).c_str(), renderSize.cx, renderSize.cy, 32);
	}

	int count = 0;

	r = pdd->QueryInterface(IID_IDirect3D2, (void**)&pd3d2);
	printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirect3D2) %p\n", r, ddResultToStr(r).c_str(), pd3d2);

	//create surfaces
	DDSURFACEDESC ddsd{ 0 };
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
		//half res
		//ddsd.dwWidth >>= 1;
		//ddsd.dwHeight >>= 1;
		//

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

	if (!pd3d2 || !pddsBack)
		return -1;

	r = pd3d2->CreateDevice(IID_IDirect3DHALDevice, pddsBack, &pd3dd2);
	printf("%d (%s) IDirect3D2::CreateDevice(IID_IDirect3DHALDevice) BACK %p\n", r, ddResultToStr(r).c_str(), pd3dd2);

	if (!pd3dd2)
		return -1;

	D3DDEVICEDESC hwCaps{sizeof(D3DDEVICEDESC)};
	D3DDEVICEDESC helCaps{sizeof(D3DDEVICEDESC)};
	r = pd3dd2->GetCaps(&hwCaps, &helCaps);
	printf("%d (%s) IDirect3DDevice2::GetCaps\n", r, ddResultToStr(r).c_str());

	r = pd3d2->CreateViewport(&pd3dViewport2, nullptr);
	printf("%d (%s) IDirect3D2::CreateViewport %p\n", r, ddResultToStr(r).c_str(), pd3dViewport2);

	if (!pd3dViewport2)
		return -1;

	r = pd3dd2->AddViewport(pd3dViewport2);
	printf("%d (%s) IDirect3DDevice2::AddViewport\n", r, ddResultToStr(r).c_str());

	DDSURFACEDESC desc{};
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
	pddsBack->GetSurfaceDesc(&desc);

	D3DVIEWPORT2 viewport{};
	viewport.dwSize = sizeof(viewport);
	viewport.dwX = 0;
	viewport.dwY = 0;
	viewport.dwWidth = desc.dwWidth;
	viewport.dwHeight = desc.dwHeight;
	viewport.dvClipX = -1;
	viewport.dvClipY = 1;
	viewport.dvClipWidth = 2;
	viewport.dvClipHeight = 2;
	viewport.dvMinZ = 0;
	viewport.dvMaxZ = 1;
	r = pd3dViewport2->SetViewport2(&viewport);
	printf("%d (%s) IDirect3DViewport2::SetViewport\n", r, ddResultToStr(r).c_str());

#if 0
	DDBLTFX ddbltfx{};
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = RGB(40, 40, 40);
	r = pddsBack->Blt(nullptr, nullptr, nullptr, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
	printf("%d (%s) IDirectDrawSurface::Blt(DDBLT_COLORFILL)\n", r, ddResultToStr(r).c_str());
#else
	D3DMATERIALHANDLE hbgMat = 0;
	IDirect3DMaterial2 *pbgMat = nullptr;
	r = pd3d2->CreateMaterial(&pbgMat, nullptr);
	printf("%d (%s) IDirect3D2::CreateMaterial %p\n", r, ddResultToStr(r).c_str(), pbgMat);
	D3DMATERIAL mat{};
	mat.dwSize = sizeof(mat);
	mat.dcvDiffuse = { .r = 0.2f, .g = 0.2f, .b = 0.2f };
	r = pbgMat->SetMaterial(&mat);
	printf("%d (%s) IDirect3DMaterial2::SetMaterial\n", r, ddResultToStr(r).c_str());
	r = pbgMat->GetHandle(pd3dd2, &hbgMat);
	printf("%d (%s) IDirect3DMaterial2::GetHandle %X\n", r, ddResultToStr(r).c_str(), hbgMat);

	r = pd3dViewport2->SetBackground(hbgMat);
	printf("%d (%s) IDirect3DViewport2::SetBackground\n", r, ddResultToStr(r).c_str());
	r = pd3dViewport2->Clear(0, nullptr, D3DCLEAR_TARGET);
	printf("%d (%s) IDirect3DViewport2::Clear(D3DCLEAR_TARGET)\n", r, ddResultToStr(r).c_str());
#endif

	D3DTLVERTEX verts[3]{};
	verts[0].color = RGBA_MAKE(250, 10, 10, 255);
	verts[0].sx = 10;
	verts[0].sy = desc.dwHeight * 0.9f;
	verts[0].sz = 0.5;
	verts[0].rhw = 1;
	verts[1].color = RGBA_MAKE(10, 250, 10, 255);
	verts[1].sx = desc.dwWidth * 0.5f;
	verts[1].sy = 10;
	verts[1].sz = 0.5;
	verts[1].rhw = 1;
	verts[2].color = RGBA_MAKE(10, 10, 250, 255);
	verts[2].sx = desc.dwWidth * 0.9f;
	verts[2].sy = desc.dwHeight * 0.8f;
	verts[2].sz = 0.5;
	verts[2].rhw = 1;

	r = pd3dd2->BeginScene();
	printf("%d (%s) IDirect3DDevice2::BeginScene\n", r, ddResultToStr(r).c_str());

	r = pd3dd2->SetCurrentViewport(pd3dViewport2);
	printf("%d (%s) IDirect3DDevice2::SetCurrentViewport\n", r, ddResultToStr(r).c_str());

	if (hwCaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)
	{
		r = pd3dd2->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTINDEPENDENT);
		printf("%d (%s) IDirect3DDevice2::SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTINDEPENDENT)\n", r, ddResultToStr(r).c_str());

		//r = pd3dd2->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_NONE);
		//sprintf("%d (%s) IDirect3DDevice2::SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_NONE)\n", r, ddResultToStr(r).c_str());
	}

	if (!(hwCaps.dwDevCaps & D3DDEVCAPS_DRAWPRIMTLVERTEX))
		printf("Warning: no hw support for DRAWPRIM TLVERTEX\n");

	r = pd3dd2->DrawPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, verts, 3, 0);
	printf("%d (%s) IDirect3DDevice2::DrawPrimitive\n", r, ddResultToStr(r).c_str());

	if (!(hwCaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT) && (hwCaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASEDGES))
	{
		r = pd3dd2->SetRenderState(D3DRENDERSTATE_EDGEANTIALIAS, TRUE);
		printf("%d (%s) IDirect3DDevice2::SetRenderState(D3DRENDERSTATE_EDGEANTIALIAS, TRUE)\n", r, ddResultToStr(r).c_str());

		WORD inds[6]{0,1, 1,2, 2,0};
		r = pd3dd2->DrawIndexedPrimitive(D3DPT_LINESTRIP, D3DVT_TLVERTEX, verts, 3, inds, 6, 0);
		printf("%d (%s) IDirect3DDevice2::DrawIndexedPrimitive\n", r, ddResultToStr(r).c_str());

		r = pd3dd2->SetRenderState(D3DRENDERSTATE_EDGEANTIALIAS, FALSE);
		printf("%d (%s) IDirect3DDevice2::SetRenderState(D3DRENDERSTATE_EDGEANTIALIAS, FALSE)\n", r, ddResultToStr(r).c_str());
	}

	r = pd3dd2->EndScene();
	printf("%d (%s) IDirect3DDevice2::EndScene\n", r, ddResultToStr(r).c_str());

	r = pd3dd2->DeleteViewport(pd3dViewport2);
	printf("%d (%s) IDirect3DDevice2::DeleteViewport\n", r, ddResultToStr(r).c_str());

	VerboseRelease(pd3dViewport2, "IDirect3DViewport2::Release");
	VerboseRelease(pbgMat, "IDirect3DMaterial2::Release");

	return 0;
}

void TestContext::draw()
{
	if (!pddsPrimary || !pddsBack)
		return;

	DDSURFACEDESC desc{};
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
	VerboseRelease(pd3dd2, "IDirect3DDevice2::Release");
	VerboseRelease(pddsBack, "IDirectDrawSurface::Release BACK");
	VerboseRelease(pddsPrimary, "IDirectDrawSurface::Release PRIMARY");
	VerboseRelease(pddClipper, "IDirectDrawClipper::Release");
	VerboseRelease(pd3d2, "IDirect3D2::Release");
	VerboseRelease(pdd, "IDirectDraw::Release");
}

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
