
#include <stdio.h>
#include <string>

#define INITGUID
#include <d3d.h>
#include "d3dmacs.h"

#pragma comment(lib, "ddraw.lib")

struct TestContext
{
	LPCTSTR class_name = TEXT("DXTestWin");
	HWND window;
	bool fullscreen = false;
	bool shouldClose = false;
	IDirectDraw *pdd = nullptr;
	IDirect3D *pd3d = nullptr;
	IDirectDrawSurface *pddsPrimary = nullptr;
	IDirectDrawSurface *pddsBack = nullptr;
	IDirectDrawClipper *pddClipper = nullptr;
	IDirect3DDevice *pd3dd = nullptr;
	IDirect3DViewport *pd3dViewport = nullptr;
	IDirect3DExecuteBuffer *pd3deb = nullptr;

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
	TestContext *ctx = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		ctx = (TestContext *)((CREATESTRUCT *)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)ctx);
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	ctx = (TestContext *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

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
	window = CreateWindowEx(exStyle, class_name, TEXT("DX2 test"), style,
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
		POINT point{ 0,0 };
		ClientToScreen(window, &point);
		RECT dstRect{};
		GetClientRect(window, &dstRect);
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

	r = pdd->QueryInterface(IID_IDirect3D, (void **)&pd3d);
	printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirect3D) %p\n", r, ddResultToStr(r).c_str(), pd3d);

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
int VerboseRelease(T *&obj, const char *msg)
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

	if (!pd3d)
		return -1;

	D3DFINDDEVICESEARCH search{};
	search.dwSize = sizeof(search);
	search.dwFlags = D3DFDS_COLORMODEL;
	search.dcmColorModel = D3DCOLOR_RGB;
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

	if (!pd3dd)
		return -1;

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
	viewport.dvScaleX = 1;
	viewport.dvScaleY = 1;
	viewport.dvMaxX = 1;
	viewport.dvMaxY = 1;
	viewport.dvMinZ = 0;
	viewport.dvMaxZ = 1;
	r = pd3dViewport->SetViewport(&viewport);
	printf("%d (%s) IDirect3DViewport::SetViewport\n", r, ddResultToStr(r).c_str());

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

		void *cur = ebDesc.lpData;

		D3DTLVERTEX verts[3]{};
		D3DTRIANGLE tri{};
		verts[0].color = RGBA_MAKE(250, 10, 10, 255);
		verts[0].sx = 10;
		verts[0].sy = 400;
		verts[0].sz = 0.5;
		verts[0].rhw = 1;
		verts[1].color = RGBA_MAKE(10, 250, 10, 255);
		verts[1].sx = 300;
		verts[1].sy = 10;
		verts[1].sz = 0.5;
		verts[1].rhw = 1;
		verts[2].color = RGBA_MAKE(10, 10, 250, 255);
		verts[2].sx = 600;
		verts[2].sy = 400;
		verts[2].sz = 0.5;
		verts[2].rhw = 1;

		tri.v1 = 0;
		tri.v2 = 1;
		tri.v3 = 2;
		tri.wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;

		VERTEX_DATA(verts, 3, cur);
		void *insStart = cur;
		OP_PROCESS_VERTICES(1, cur);
		PROCESSVERTICES_DATA(D3DPROCESSVERTICES_COPY | D3DPROCESSVERTICES_UPDATEEXTENTS, 0, 3, cur);
		if (QWORD_ALIGNED(cur)) {
			OP_NOP(cur);
		}
		OP_TRIANGLE_LIST(1, cur);
		TRIANGLE_LIST_DATA(&tri, 1, cur);
		OP_EXIT(cur);

		r = pd3deb->Unlock();
		printf("%d (%s) IDirect3DExecuteBuffer::Unlock\n", r, ddResultToStr(r).c_str());

		D3DEXECUTEDATA exData{};
		exData.dwSize = sizeof(exData);
		exData.dwVertexCount = 3;
		exData.dwVertexOffset = 0;
		exData.dwInstructionOffset = ((char *)insStart - (char *)ebDesc.lpData);
		exData.dwInstructionLength = ((char *)cur - (char *)insStart);
		r = pd3deb->SetExecuteData(&exData);
		printf("%d (%s) IDirect3DExecuteBuffer::SetExecuteData\n", r, ddResultToStr(r).c_str());
	}

	r = pd3dd->BeginScene();
	printf("%d (%s) IDirect3DDevice::BeginScene\n", r, ddResultToStr(r).c_str());

	r = pd3dd->Execute(pd3deb, pd3dViewport, D3DEXECUTE_UNCLIPPED);
	printf("%d (%s) IDirect3DDevice::Execute\n", r, ddResultToStr(r).c_str());

	r = pd3dd->EndScene();
	printf("%d (%s) IDirect3DDevice::EndScene\n", r, ddResultToStr(r).c_str());

	r = pd3dd->DeleteViewport(pd3dViewport);
	printf("%d (%s) IDirect3DDevice::DeleteViewport\n", r, ddResultToStr(r).c_str());

	VerboseRelease(pd3dViewport, "IDirect3DViewport::Release");
	VerboseRelease(pd3deb, "IDirect3DExecuteBuffer::Release");

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
	RECT srcRect{ 0, 0, desc.dwWidth, desc.dwHeight };

	RECT dstRect{};
	POINT point{ 0, 0 };
	ClientToScreen(window, &point);
	GetClientRect(window, &dstRect);
	OffsetRect(&dstRect, point.x, point.y);

	pddsPrimary->Blt(&dstRect, pddsBack, &srcRect, DDBLT_WAIT, NULL);
}

void TestContext::shutdownDD()
{
	VerboseRelease(pd3dd, "IDirect3DDevice::Release");
	VerboseRelease(pddsBack, "IDirectDrawSurface::Release BACK");
	VerboseRelease(pddsPrimary, "IDirectDrawSurface::Release PRIMARY");
	VerboseRelease(pddClipper, "IDirectDrawClipper::Release");
	VerboseRelease(pd3d, "IDirect3D::Release");
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
