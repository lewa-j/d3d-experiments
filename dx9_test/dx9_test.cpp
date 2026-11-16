
#define D3D_DISABLE_9EX 1

#include <stdio.h>
#include <inttypes.h>
#include <string>

#include <d3d9.h>

#define Log printf

std::string d3dErrorString(HRESULT r)
{
	if (r == D3D_OK)
		return "OK";

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

	if (!lpMsgBuf)
	{
		int sev = (r >> 31) & 1;
		int fac = (r >> 16) & 0x7FFF;
		int code = r & 0xFFFF;
		char buff[256] = { 0 };
		snprintf(buff, 255, "%d 0x%X %d", sev, fac, code);
		buff[255] = 0;
		return std::string(buff);
	}
	std::string ret(lpMsgBuf);
	LocalFree(lpMsgBuf);
	return ret;
}


int printAdapter(IDirect3D9 *d3d, UINT adapter)
{
	HRESULT r = 0;

	D3DADAPTER_IDENTIFIER9 id{};
	r = d3d->GetAdapterIdentifier(adapter, 0, &id);

	uint64_t guid[2];
	ZeroMemory(&guid, sizeof(guid));
	memcpy(guid, &id.DeviceIdentifier, sizeof(GUID));

	Log("%d GetAdapterIdentifier(%d) Driver \"%s\" Description \"%s\" DeviceName \"%s\"\n"
		"   DriverVersion %d.%d.%d.%d\n"
		"   VendorId 0x%X DeviceId 0x%X SubSysId 0x%X Revision 0x%X\n"
		"   DeviceIdentifier %.16" PRIX64 "%.16" PRIX64 " WHQLLevel %d\n",
		r, adapter, id.Driver, id.Description, id.DeviceName,
		HIWORD(id.DriverVersion.HighPart), LOWORD(id.DriverVersion.HighPart), HIWORD(id.DriverVersion.LowPart), LOWORD(id.DriverVersion.LowPart),
		id.VendorId, id.DeviceId, id.SubSysId, id.Revision,
		guid[0], guid[1], id.WHQLLevel
	);

	D3DDISPLAYMODE mode{};
	r = d3d->GetAdapterDisplayMode(adapter, &mode);
	Log("%d GetAdapterDisplayMode(%d) %dx%d@%d fmt %d\n", r, adapter, mode.Width, mode.Height, mode.RefreshRate, mode.Format);

	int modeCount = d3d->GetAdapterModeCount(adapter, mode.Format);
	Log("AdapterModeCount(%d) %d\n", adapter, modeCount);
	for(int mi = 0; mi < modeCount; mi++)
	{
		D3DDISPLAYMODE dm{};
		r = d3d->EnumAdapterModes(adapter, mode.Format, mi, &dm);
		Log("%d EnumAdapterModes(%d, %d) \t%dx%d@%d \tfmt %d\n", r, adapter, mi, dm.Width, dm.Height, dm.RefreshRate, dm.Format);
	}

	const char *d3ddevtype_str[5]{
		"?",
		"HAL",
		"REF",
		"SW",
		"NULLREF",
	};

	for (int dt = D3DDEVTYPE_HAL; dt <= D3DDEVTYPE_NULLREF; dt++)
	{
		r = d3d->CheckDeviceType(adapter, (D3DDEVTYPE)dt, mode.Format, mode.Format, true);
		Log("%d CheckDeviceType(%d, %s, %d)\n", r, adapter, d3ddevtype_str[dt], mode.Format);
	}

	D3DCAPS9 caps{};
	r = d3d->GetDeviceCaps(adapter, D3DDEVTYPE_HAL, &caps);
	Log("%d GetDeviceCaps(%d, HAL)\n", r, adapter);

	Log(" MaxTexture %dx%d\n", caps.MaxTextureWidth, caps.MaxTextureHeight);
	Log(" VertexShaderVersion 0x%X PixelShaderVersion 0x%X\n", caps.VertexShaderVersion, caps.PixelShaderVersion);
	Log(" NumberOfAdaptersInGroup %d\n", caps.NumberOfAdaptersInGroup);

	return 0;
}

static bool shouldClose = false;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NCCREATE)
	{
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
	case WM_DESTROY:
		shouldClose = true;
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
	{
		//Render();
		ValidateRect(hWnd, nullptr);
		return 0;
	}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

struct vert_t {
	float x, y, z, rhw;
	D3DCOLOR color;
};

int main()
{
	IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
	Log("Direct3DCreate9 %p\n", d3d);
	if (!d3d)
		return -1;

	int adapterCount = d3d->GetAdapterCount();
	Log("AdapterCount %d\n", adapterCount);
	for (int ai = 0; ai < adapterCount; ai++)
	{
		printAdapter(d3d, ai);
	}

	// window
	DWORD style = WS_OVERLAPPEDWINDOW;
	DWORD exStyle = WS_EX_APPWINDOW;
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L,
					  GetModuleHandle(nullptr), NULL, NULL, NULL, NULL,
					  TEXT("D3D9TestWin"), NULL };
	RegisterClassEx(&wc);

	RECT winSize = { 0,0,640,480 };
	AdjustWindowRectEx(&winSize, style, false, exStyle);

	HWND wnd = CreateWindowEx(exStyle, TEXT("D3D9TestWin"), TEXT("D3D9 Test"),
		style, CW_USEDEFAULT, CW_USEDEFAULT,
		winSize.right - winSize.left, winSize.bottom - winSize.top,
		GetDesktopWindow(), NULL, wc.hInstance, nullptr);
	printf("CreateWindowEx = %p\n", wnd);
	ShowWindow(wnd, SW_SHOW);

	D3DPRESENT_PARAMETERS present{};
	present.Windowed = true;
	present.SwapEffect = D3DSWAPEFFECT_DISCARD;

	IDirect3DDevice9 *d3dd = nullptr;
	HRESULT r = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &present, &d3dd);
	Log("%d(%s) CreateDevice(HAL, HW VP) %p\n", r, d3dErrorString(r).c_str(), d3dd);

	if (!d3dd)
	{
		int c = d3d->Release();
		Log("D3D Release %d\n", c);
		return -1;
	}

	Log("PRESENT_PARAMETERS %dx%d fmt %d count %d\n", present.BackBufferWidth, present.BackBufferHeight,
		present.BackBufferFormat, present.BackBufferCount);

	const DWORD fvf_vert = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
	vert_t verts[6] = {
	{ 150, 50, 0.1, 1, 0xFFFF0000 },
	{ 250, 250, 0.1, 1, 0xFF00FF00 },
	{ 50, 250, 0.1, 1, 0xFF0000FF }
	};

	d3dd->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 64), 1, 0);
	d3dd->BeginScene();
	d3dd->SetFVF(fvf_vert);
	d3dd->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, verts, sizeof(vert_t));
	d3dd->EndScene();
	r = d3dd->Present(nullptr, nullptr, nullptr, nullptr);
	Log("%d Present\n", r);

	while (!shouldClose)
	{
		MSG msg;
		ZeroMemory(&msg, sizeof(msg));

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			//while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (GetAsyncKeyState(VK_ESCAPE))
			break;
		
		//r = d3dd->Present(nullptr, nullptr, nullptr, nullptr);
	}

	int c = d3dd->Release();
	Log("D3DD Release %d\n", c);

	c = d3d->Release();
	Log("D3D Release %d\n", c);

	return 0;
}
