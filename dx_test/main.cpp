
#define INITGUID

#include <ddraw.h>
#include <d3d.h>
//#include <d3d8.h>
#include <stdio.h>

void printResult(HRESULT r)
{
	LPSTR msgBuf = nullptr;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
		r, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPSTR)&msgBuf, 0, nullptr);

	if (msgBuf)
	{
		printf("%s", msgBuf);
		LocalFree(msgBuf);
		return;
	}

	if (r == DDERR_NOCOOPERATIVELEVELSET)
	{
		printf("DDERR_NOCOOPERATIVELEVELSET\n");
		return;
	}

	printf("%d 0x%X %d\n", (r >> 31) & 1, (r >> 16) & 0x7FFF, r & 0xFFFF);
}

LRESULT WINAPI WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

void printRect(const char* name, const RECT& r)
{
	printf("%s %d %d %d %d\n", name, r.left, r.top, r.right, r.bottom);

}

void createD3D_2_6()
{
	HINSTANCE inst = GetModuleHandleA(nullptr);
	WNDCLASSA wcls{ CS_HREDRAW | CS_VREDRAW, WinProc, 0, 0, inst, 0, 0, 0, nullptr, "DXTestWin" };
	RegisterClassA(&wcls);

	RECT winRect{ 0,0,640,480 };
	int winStyle = WS_OVERLAPPEDWINDOW;
	int winExStyle = WS_EX_APPWINDOW;
	AdjustWindowRectEx(&winRect, winStyle, false, winExStyle);
	printRect("AdjustWindowRectEx", winRect);

	int winWidth = winRect.right - winRect.left;
	int winHeight = winRect.bottom - winRect.top;
	HWND win = CreateWindowExA(winExStyle, "DXTestWin", "Direct3D", winStyle | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, winWidth, winHeight, nullptr, nullptr, inst, nullptr);

	RECT clrect{};
	GetClientRect(win, &clrect);
	printRect("GetClientRect", clrect);
	winWidth = clrect.right - clrect.left;
	winHeight = clrect.bottom - clrect.top;
	HRESULT r = 0;
#if 0
	r = DirectDrawEnumerateA([](GUID FAR* guid, LPSTR driverDesc, LPSTR driverName, LPVOID pUser) -> int
		{
			printf(" DirectDrawEnumerate: %lX %s %s\n", guid ? guid->Data1 : 0, driverName, driverDesc);
			return DDENUMRET_OK;
		}, nullptr);
	if (r)
	{
		printf("%d DirectDrawEnumerateA ", r);
		printResult(r);
	}
#endif
	IDirectDraw* pdd = nullptr;
	r = DirectDrawCreate(nullptr, &pdd, nullptr);
	printf("%d DirectDrawCreate %p\n", r, pdd);
	if (r != DD_OK || !pdd)
		return;

	r = pdd->SetCooperativeLevel(win, DDSCL_NORMAL);
	printf("%d DD::SetCooperativeLevel(DDSCL_NORMAL)\n", r);

	IDirectDrawSurface* pddsPrime = nullptr;
	{
		DDSURFACEDESC ddsd{ sizeof(DDSURFACEDESC), DDSD_CAPS, 0 };
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		r = pdd->CreateSurface(&ddsd, &pddsPrime, nullptr);
		printf("%d DD::CreateSurface primary %p\n", r, pddsPrime);
		if (r) printResult(r);

		DDSURFACEDESC desc{};
		desc.dwSize = sizeof(desc);
		desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
		pddsPrime->GetSurfaceDesc(&desc);
		printf("primary %dx%d\n", desc.dwWidth, desc.dwHeight);
	}

	IDirectDrawClipper *pddClipper = nullptr;
	r = pdd->CreateClipper(0, &pddClipper, nullptr);
	printf("%d DD::CreateClipper %p\n", r, pddClipper);

	r = pddClipper->SetHWnd(0, win);
	printf("%d DDClipper::SetHWnd\n", r);

	r = pddsPrime->SetClipper(pddClipper);
	printf("%d DDS::SetClipper\n", r);

	IDirectDrawSurface* pdds = nullptr;
	{
		DDSURFACEDESC ddsd{ sizeof(DDSURFACEDESC), DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT, winHeight, winWidth };
		ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_OFFSCREENPLAIN;
		r = pdd->CreateSurface(&ddsd, &pdds, nullptr);
		printf("%d DD::CreateSurface back %p\n", r, pdds);
		if (r) printResult(r);
	}

	if (0)
	{
	IDirect3D* pd3d = nullptr;
	r = pdd->QueryInterface(IID_IDirect3D, (void**)&pd3d);
	printf("%d QueryInterface(IID_IDirect3D) %p\n", r, pd3d);
	if (r)
	{
		printResult(r);
		printf("DX2 D3D unavailable\n");
	}
	if (pd3d)
	{
		printf("DX2 D3D\n");
		r = pd3d->EnumDevices([](GUID FAR* lpGuid, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC descHW, LPD3DDEVICEDESC descHEL, LPVOID pUser) -> HRESULT
			{
				printf(" EnumDevices: %lX %s %s flags %X cm %d\n", lpGuid->Data1, lpDeviceName, lpDeviceDescription, descHW->dwFlags, descHW->dcmColorModel);
				return DDENUMRET_OK;
			}, nullptr);
		if (r) printf("%d D3D::EnumDevices\n", r);
		pd3d->Release();
	}
	}

	{
	IDirect3D2* pd3d2 = nullptr;
	r = pdd->QueryInterface(IID_IDirect3D2, (void**)&pd3d2);
	printf("%d QueryInterface(IID_IDirect3D2) %p\n", r, pd3d2);
	if (r)
	{
		printResult(r);
		printf("DX5 D3D unavailable\n");
	}
	if (pd3d2)
	{
		printf("DX5 D3D\n");
		r = pd3d2->EnumDevices([](GUID FAR* lpGuid, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC descHW, LPD3DDEVICEDESC descHEL, LPVOID) -> HRESULT
			{
				printf(" EnumDevices: %lX %s %s flags %X cm %d\n", lpGuid->Data1, lpDeviceName, lpDeviceDescription, descHW->dwFlags, descHW->dcmColorModel);
				return DDENUMRET_OK;
			}, nullptr);
		if (r) printf("%d D3D2::EnumDevices\n", r);

		IDirect3DDevice2 *pd3dd2 = nullptr;
		r = pd3d2->CreateDevice(IID_IDirect3DHALDevice, pdds, &pd3dd2);
		printf("%d D3D2::CreateDevice %p\n", r, pd3dd2);

		IDirect3DViewport2 *pd3dv2 = nullptr;
		r = pd3d2->CreateViewport(&pd3dv2, nullptr);
		printf("%d D3D2::CreateViewport %p\n", r, pd3dv2);

		r = pd3dd2->AddViewport(pd3dv2);
		printf("%d D3DD2::AddViewport\n", r);

		D3DVIEWPORT vp{};
		vp.dwSize = sizeof(vp);
		vp.dwX = 0;
		vp.dwY = 0;
		vp.dwWidth = winWidth;
		vp.dwHeight = winHeight;
		vp.dvScaleX = winWidth/2;
		vp.dvScaleY = winHeight/2;
		vp.dvMaxX = 1;
		vp.dvMaxY = 1;
		r = pd3dv2->SetViewport(&vp);
		printf("%d D3DV2::SetViewport\n", r);
		if (r) printResult(r);

		r = pd3dd2->BeginScene();
		printf("%d D3DD2::BeginScene\n", r);

		r = pd3dd2->SetCurrentViewport(pd3dv2);
		printf("%d D3DD2::SetCurrentViewport\n", r);

		D3DTLVERTEX verts[3]{0};
		verts[0].color = RGBA_MAKE(250, 10, 10, 255);
		verts[0].sx = 10;
		verts[0].sy = 10;
		verts[0].rhw = 1;
		verts[1].color = RGBA_MAKE(10, 250, 10, 255);
		verts[1].sx = 100;
		verts[1].sy = 10;
		verts[1].rhw = 1;
		verts[2].color = RGBA_MAKE(10, 10, 250, 255);
		verts[2].sx = 100;
		verts[2].sy = 100;
		verts[2].rhw = 1;

		r = pd3dd2->DrawPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, verts, 3, 0);
		printf("%d D3DD2::DrawPrimitive\n", r);

		r = pd3dd2->EndScene();
		printf("%d D3DD2::EndScene\n", r);

		MSG msg = {};
		while (GetMessageA(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);

			RECT dstRect{};
			GetClientRect(win, &dstRect);
			POINT point{0,0};
			ClientToScreen(win, &point);
			OffsetRect(&dstRect, point.x, point.y);

			RECT srcRect{ 0,0, winWidth, winHeight };
			//printRect("src", srcRect);
			//printRect("dst", dstRect);
			r = pddsPrime->Blt(&dstRect, pdds, &srcRect, DDBLT_WAIT, nullptr);

			if (r)
			{
				printf("%d Blt\n", r);
				printResult(r);
				break;
			}
		}

		r = pd3dd2->DeleteViewport(pd3dv2);
		printf("%d D3DD2::DeleteViewport\n", r);

		ULONG c = pd3dv2->Release();
		printf("D3DV2::Release %lu\n", c);

		c = pd3dd2->Release();
		printf("D3DD2::Release %lu\n", c);

		c = pd3d2->Release();
		printf("D3D2::Release %lu\n", c);
	}
	}
#if 0
	{
	IDirect3D3* pd3d3 = nullptr;
	r = pdd->QueryInterface(IID_IDirect3D3, (void**)&pd3d3);
	printf("%d QueryInterface(IID_IDirect3D3) %p\n", r, pd3d3);
	if (r)
	{
		printResult(r);
		printf("DX6 D3D unavailable\n");
	}
	if (pd3d3)
	{
		printf("DX6 D3D\n");
		pd3d3->Release();
	}
	}
#endif
	ULONG c = pdd->Release();
	printf("IDirectDraw Release %lu\n", c);
	
}

void createD3D7()
{
	IDirectDraw7* pdd7 = nullptr;
	HRESULT r = DirectDrawCreateEx(nullptr, (void**)&pdd7, IID_IDirectDraw7, nullptr);
	printf("%d DirectDrawCreateEx(IID_IDirectDraw7) %p\n", r, pdd7);
	if (!pdd7)
		return;

	IDirect3D7* pd3d7 = nullptr;
	r = pdd7->QueryInterface(IID_IDirect3D7, (void**)&pd3d7);
	printf("%d QueryInterface(IID_IDirect3D7) %p\n", r, pd3d7);
	if (r)
	{
		printResult(r);
		printf("DX7 D3D unavailable\n");
	}
	if (pd3d7)
		pd3d7->Release();

	ULONG c = pdd7->Release();
	printf("IDirectDraw7 Release %lu\n", c);
}

void createD3D8()
{
	HMODULE lib = LoadLibraryA("d3d8.dll");
	printf("d3d8.dll %p\n", lib);
	if (!lib)
	{
		printf("GetLastError %u\n", GetLastError());
		return;
	}

	struct IDirect3D8;
	typedef IDirect3D8 *(WINAPI* PFN_Direct3DCreate8)(UINT SDKVersion);

	PFN_Direct3DCreate8 Direct3DCreate8 = (PFN_Direct3DCreate8)GetProcAddress(lib, "Direct3DCreate8");
	printf("%p Direct3DCreate8 PFN\n", Direct3DCreate8);

	const UINT D3D_SDK_VERSION = 220;

	IDirect3D8* pd3d8 = Direct3DCreate8(D3D_SDK_VERSION);
	printf("Direct3DCreate8 %p\n", pd3d8);
#if 0
	if (pd3d8)
	{
		ULONG c = pd3d8->Release();
		printf("IDirect3D8 Release %lu\n", c);
	}
#endif
}

int main()
{
	printf("dx_test\n");

	createD3D_2_6();

	createD3D7();

	//createD3D8();

	return 0;
}