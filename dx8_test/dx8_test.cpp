
#include <stdio.h>
#include "d3d8.h"
#include <string>
#include <math.h>

#if (_MSC_VER > 1200)//TODO test newer versions
#include <stdint.h>
#include <inttypes.h>
#else
#define uint64_t unsigned __int64
#define uint32_t unsigned __int32
#define PRIX64 "I64x"
#define nullptr NULL
#define snprintf _snprintf
#endif

#define Log printf

static void printAdapter(IDirect3D8 *d3d, UINT adapterIndex)
{
	D3DADAPTER_IDENTIFIER8 ident;
	ZeroMemory(&ident, sizeof(ident));
	HRESULT r = d3d->GetAdapterIdentifier(adapterIndex, 0/*D3DENUM_NO_WHQL_LEVEL*/, &ident);

	uint64_t guid[2];
	ZeroMemory(&guid, sizeof(guid));
	memcpy(guid, &ident.DeviceIdentifier, sizeof(GUID));

	Log("%d GetAdapterIdentifier(%d) Driver \"%s\" Description \"%s\"\n"
		"   DriverVersion %d.%d.%d.%d\n"
		"   VendorId 0x%X DeviceId 0x%X SubSysId 0x%X Revision 0x%X\n"
		"   DeviceIdentifier %.16" PRIX64 "%.16" PRIX64 " WHQLLevel %d\n",
		r, adapterIndex, ident.Driver, ident.Description,
		HIWORD(ident.DriverVersion.HighPart), LOWORD(ident.DriverVersion.HighPart), HIWORD(ident.DriverVersion.LowPart), LOWORD(ident.DriverVersion.LowPart),
		ident.VendorId, ident.DeviceId, ident.SubSysId, ident.Revision,
		guid[0], guid[1], ident.WHQLLevel
		);
	UINT modeCount = d3d->GetAdapterModeCount(adapterIndex);
	Log("GetAdapterModeCount %u\n", modeCount);
	for (UINT mi = 0; mi < modeCount; mi++)
	{
		D3DDISPLAYMODE mode;
		ZeroMemory(&mode, sizeof(mode));
		r = d3d->EnumAdapterModes(adapterIndex, mi, &mode);
		Log("%d EnumAdapterModes(%d, %d) %dx%d@%d fmt %d\n", r, adapterIndex, mi,
			mode.Width, mode.Height, mode.RefreshRate, mode.Format);
	}
	D3DDISPLAYMODE mode;
	ZeroMemory(&mode, sizeof(mode));
	r = d3d->GetAdapterDisplayMode(adapterIndex, &mode);
	Log("%d GetAdapterDisplayMode(%d) %dx%d@%d fmt %d\n", r, adapterIndex,
		mode.Width, mode.Height, mode.RefreshRate, mode.Format);
	HMONITOR hm = d3d->GetAdapterMonitor(adapterIndex);
	Log("GetAdapterMonitor(%d) %p\n", adapterIndex, hm);

	D3DCAPS8 caps;
	ZeroMemory(&caps, sizeof(caps));
	r = d3d->GetDeviceCaps(adapterIndex, D3DDEVTYPE_HAL, &caps);
	Log("%d GetDeviceCaps(%d HAL)\n"
		"   DeviceType %d AdapterOrdinal %d\n"
		"   DevCaps 0x%X\n"
		"   MaxTextureSize %dx%dx%d\n"
		"   FVFCaps 0x%X TextureOpCaps 0x%X MaxTextureBlendStages %d MaxSimultaneousTextures %d\n"
		"   VertexProcessingCaps 0x%X MaxActiveLights %d MaxUserClipPlanes %d MaxVertexBlendMatrices %d MaxVertexBlendMatrixIndex %d\n"
		"   MaxPointSize %g MaxPrimitiveCount %d MaxVertexIndex %d MaxStreams %d MaxStreamStride %d\n"
		"   VertexShaderVersion 0x%X MaxVertexShaderConst %d\n"
		"   PixelShaderVersion 0x%X MaxPixelShaderValue %g\n",
		r, adapterIndex,
		caps.DeviceType, caps.AdapterOrdinal,
		caps.DevCaps,
		caps.MaxTextureWidth, caps.MaxTextureHeight, caps.MaxVolumeExtent,
		caps.FVFCaps, caps.TextureOpCaps, caps.MaxTextureBlendStages, caps.MaxSimultaneousTextures,
		caps.VertexProcessingCaps, caps.MaxActiveLights, caps.MaxUserClipPlanes, caps.MaxVertexBlendMatrices, caps.MaxVertexBlendMatrixIndex,
		caps.MaxPointSize, caps.MaxPrimitiveCount, caps.MaxVertexIndex, caps.MaxStreams, caps.MaxStreamStride,
		caps.VertexShaderVersion, caps.MaxVertexShaderConst,
		caps.PixelShaderVersion, caps.MaxPixelShaderValue);
}

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
		char buff[256] = {0};
		snprintf(buff, 255, "%d 0x%X %d", sev, fac, code);
		buff[255] = 0;
		return std::string(buff);
	}
	std::string ret(lpMsgBuf);
	LocalFree(lpMsgBuf);
	return ret;
}

struct d3d8TestContext
{

};

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	d3d8TestContext *ctx = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		ctx = (d3d8TestContext*)((CREATESTRUCT*)lParam)->lpCreateParams;

#ifndef SetWindowLongPtr
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)ctx);
#else
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)ctx);
#endif
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

#ifndef SetWindowLongPtr
	ctx = (d3d8TestContext *)GetWindowLong(hWnd, GWL_USERDATA);
#else
	ctx = (d3d8TestContext *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#endif

	switch (uMsg)
	{
	case WM_DESTROY:
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

int main(int argc, const char **argv)
{
	HMODULE lib = LoadLibraryA("d3d8.dll");
	Log("d3d8.dll %p\n", lib);
	if (!lib)
	{
		printf("GetLastError %u\n", GetLastError());
		return -1;
	}

	typedef IDirect3D8 *(WINAPI* PFN_Direct3DCreate8)(UINT SDKVersion);
	PFN_Direct3DCreate8 Direct3DCreate8 = (PFN_Direct3DCreate8)GetProcAddress(lib, "Direct3DCreate8");
	printf("%p Direct3DCreate8 PFN\n", Direct3DCreate8);

	IDirect3D8 *d3d = Direct3DCreate8(D3D_SDK_VERSION);

	Log("Direct3DCreate8 %p\n", d3d);
	if (!d3d)
		return -1;

	UINT adapterCount = d3d->GetAdapterCount();
	Log("AdapterCount %u\n", adapterCount);

	for (UINT ai = 0; ai < adapterCount; ai++)
	{
		printAdapter(d3d, ai);
	}

	d3d8TestContext ctx;
	ZeroMemory(&ctx, sizeof(ctx));

	// window
	DWORD style = WS_OVERLAPPEDWINDOW;
	//if (fullscreen)
	//	style = WS_POPUP;
	DWORD exStyle = WS_EX_APPWINDOW;
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L,
					  GetModuleHandle(nullptr), NULL, NULL, NULL, NULL,
					  TEXT("D3D8TestWin"), NULL };
	RegisterClassEx(&wc);

	RECT winSize = { 0,0,640,480 };
	AdjustWindowRectEx(&winSize, style, false, exStyle);

	HWND hWnd = CreateWindowEx(exStyle, TEXT("D3D8TestWin"), TEXT("D3D8 Test"),
		style, CW_USEDEFAULT, CW_USEDEFAULT,
		winSize.right - winSize.left, winSize.bottom - winSize.top,
		GetDesktopWindow(), NULL, wc.hInstance, &ctx);
	printf("CreateWindowEx = %p\n", hWnd);
	ShowWindow(hWnd, SW_SHOW);

	D3DDISPLAYMODE mode;
	ZeroMemory(&mode, sizeof(mode));
	HRESULT r = d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
	Log("%d (%s) GetAdapterDisplayMode\n", r, d3dErrorString(r).c_str());

	r = d3d->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mode.Format, mode.Format, true);
	Log("%d (%s) CheckDeviceType(HAL, fmt %d)\n", r, d3dErrorString(r).c_str(), mode.Format);

	D3DPRESENT_PARAMETERS presentParams;
	ZeroMemory(&presentParams, sizeof(presentParams));
	presentParams.BackBufferWidth = 640;
	presentParams.BackBufferHeight = 480;
	presentParams.BackBufferFormat = mode.Format;
	presentParams.BackBufferCount = 1;
	presentParams.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
	presentParams.Windowed = true;

	IDirect3DDevice8 *d3dd = nullptr;
	r = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParams, &d3dd);
	Log("%d (%s) IDirect3D8::CreateDevice(HARDWARE_VERTEXPROCESSING) %p BackBufferCount %d\n", r, d3dErrorString(r).c_str(), d3dd, presentParams.BackBufferCount);
	if (!d3dd)
	{
		Log("CreateDevice with D3DCREATE_HARDWARE_VERTEXPROCESSING failed\n");
		r = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParams, &d3dd);
		Log("%d (%s) IDirect3D8::CreateDevice(SOFTWARE_VERTEXPROCESSING) %p BackBufferCount %d\n", r, d3dErrorString(r).c_str(), d3dd, presentParams.BackBufferCount);
		if (!d3dd)
		{
			Log("CreateDevice with D3DCREATE_SOFTWARE_VERTEXPROCESSING failed\n");
			return -1;
		}
	}

	DWORD decl[] = {
		D3DVSD_STREAM(0),
		D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),
		D3DVSD_SKIP(1),
		D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR),
		D3DVSD_END()
	};
	DWORD func[] = {
		D3DVS_VERSION(1,0),
		/*
		//mov oPos, v0
		D3DSIO_MOV,
		D3DSRO_POSITION | D3DSP_WRITEMASK_ALL | D3DSPR_RASTOUT | 0x80000000,
		D3DVSDE_POSITION | D3DVS_NOSWIZZLE | D3DSPR_INPUT | 0x80000000,
		*/
#if 0
		/* transposed mtx
		dp4 r0.x, v0, c0
		dp4 r0.y, v0, c1
		dp4 r0.z, v0, c2
		dp4 r0.w, v0, c3
		*/
		D3DSIO_DP4,
		0 | D3DSP_WRITEMASK_0 | D3DSPR_TEMP | 0x80000000,
		D3DVSDE_POSITION | D3DVS_NOSWIZZLE | D3DSPR_INPUT | 0x80000000,
		0 | D3DVS_NOSWIZZLE | D3DSPR_CONST | 0x80000000,
		D3DSIO_DP4,
		0 | D3DSP_WRITEMASK_1 | D3DSPR_TEMP | 0x80000000,
		D3DVSDE_POSITION | D3DVS_NOSWIZZLE | D3DSPR_INPUT | 0x80000000,
		1 | D3DVS_NOSWIZZLE | D3DSPR_CONST | 0x80000000,
		D3DSIO_DP4,
		0 | D3DSP_WRITEMASK_2 | D3DSPR_TEMP | 0x80000000,
		D3DVSDE_POSITION | D3DVS_NOSWIZZLE | D3DSPR_INPUT | 0x80000000,
		2 | D3DVS_NOSWIZZLE | D3DSPR_CONST | 0x80000000,
		D3DSIO_DP4,
		0 | D3DSP_WRITEMASK_3 | D3DSPR_TEMP | 0x80000000,
		D3DVSDE_POSITION | D3DVS_NOSWIZZLE | D3DSPR_INPUT | 0x80000000,
		3 | D3DVS_NOSWIZZLE | D3DSPR_CONST | 0x80000000,
#endif
		/*
		mul r0, c0, v0.x
		mad r0, c1, v0.y, r0 
		mad r0, c2, v0.z, r0
		mad r0, c3, v0.w, r0
		*/
		D3DSIO_MUL,
		0 | D3DSP_WRITEMASK_ALL | D3DSPR_TEMP | 0x80000000,
		0 | D3DVS_NOSWIZZLE | D3DSPR_CONST | 0x80000000,
		D3DVSDE_POSITION | D3DVS_X_X | D3DVS_Y_X | D3DVS_Z_X | D3DVS_W_X | D3DSPR_INPUT | 0x80000000,
		D3DSIO_MAD,
		0 | D3DSP_WRITEMASK_ALL | D3DSPR_TEMP | 0x80000000,
		1 | D3DVS_NOSWIZZLE | D3DSPR_CONST | 0x80000000,
		D3DVSDE_POSITION | D3DVS_X_Y | D3DVS_Y_Y | D3DVS_Z_Y | D3DVS_W_Y | D3DSPR_INPUT | 0x80000000,
		0 | D3DVS_NOSWIZZLE | D3DSPR_TEMP | 0x80000000,
		D3DSIO_MAD,
		0 | D3DSP_WRITEMASK_ALL | D3DSPR_TEMP | 0x80000000,
		2 | D3DVS_NOSWIZZLE | D3DSPR_CONST | 0x80000000,
		D3DVSDE_POSITION | D3DVS_X_Z | D3DVS_Y_Z | D3DVS_Z_Z | D3DVS_W_Z | D3DSPR_INPUT | 0x80000000,
		0 | D3DVS_NOSWIZZLE | D3DSPR_TEMP | 0x80000000,
		D3DSIO_MAD,
		0 | D3DSP_WRITEMASK_ALL | D3DSPR_TEMP | 0x80000000,
		3 | D3DVS_NOSWIZZLE | D3DSPR_CONST | 0x80000000,
		D3DVSDE_POSITION | D3DVS_X_W | D3DVS_Y_W | D3DVS_Z_W | D3DVS_W_W | D3DSPR_INPUT | 0x80000000,
		0 | D3DVS_NOSWIZZLE | D3DSPR_TEMP | 0x80000000,

		//mov oPos, r0
		D3DSIO_MOV,
		D3DSRO_POSITION | D3DSP_WRITEMASK_ALL | D3DSPR_RASTOUT | 0x80000000,
		0 | D3DVS_NOSWIZZLE | D3DSPR_TEMP | 0x80000000,
#if 0
		//mov oD0, c4
		D3DSIO_MOV,
		0 | D3DSP_WRITEMASK_ALL | D3DSPR_ATTROUT | 0x80000000,
		4 | D3DVS_NOSWIZZLE | D3DSPR_CONST | 0x80000000,
#else
		//mov oD0, v1
		D3DSIO_MOV,
		0 | D3DSP_WRITEMASK_ALL | D3DSPR_ATTROUT | 0x80000000,
		//D3DVSDE_DIFFUSE | D3DVS_NOSWIZZLE | D3DSPR_INPUT | 0x80000000,
		0 | D3DVS_NOSWIZZLE | D3DSPR_TEMP | 0x80000000,
#endif
		D3DVS_END()
	};
	DWORD vertShader = 0;
	r = d3dd->CreateVertexShader(decl, func, &vertShader, 0);
	Log("%d (%s) IDirect3DDevice8::CreateVertexShader %d\n", r, d3dErrorString(r).c_str(), vertShader);

#if 0
	struct vert_t {
		float x, y, z, rhw;
		D3DCOLOR color;
	};
	//const DWORD fvf_vert = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
	const DWORD fvf_vert = D3DFVF_XYZ | D3DFVF_PSIZE | D3DFVF_DIFFUSE;
	const int vertCount = 6;
#else
	struct vert2_t {
		float x, y, z;
		D3DCOLOR color;
		float s, t;
	};
	const DWORD fvf_vert = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	const int vertCount = 3;
#endif
#define USE_VBO 0
#if USE_VBO
	IDirect3DVertexBuffer8 *vbo = nullptr;
	r = d3dd->CreateVertexBuffer(sizeof(vert_t) * vertCount, 0, fvf_vert, D3DPOOL_DEFAULT, &vbo);
	Log("%d CreateVertexBuffer %p\n", r, vbo);

	//todo vert2
	vert_t *pv = nullptr;
	vbo->Lock(0, sizeof(vert_t) * vertCount, (BYTE **)&pv, 0);
	pv[0] = {150, 50, 0.5, 1, 0xFFFF0000};
	pv[1] = {250, 250, 0.5, 1, 0xFF00FF00};
	pv[2] = {50, 250, 0.5, 1, 0xFF0000FF};
	pv[3] = { 0, -0.5, 0.5, 1, 0xFFFF0000 };
	pv[4] = { 0.5, 0.5, 0.5, 1, 0xFF00FF00 };
	pv[5] = { -0.5, 0.5, 0.5, 1, 0xFF0000FF };
	vbo->Unlock();
#else
#if 0
	vert_t verts[6] = {
		{150, 50, 0.1, 1, 0xFFFF0000},
		{250, 250, 0.1, 1, 0xFF00FF00},
		{50, 250, 0.1, 1, 0xFF0000FF},

		{0, 0.5, 0.1, 1, 0xFFFF0000},
		{0.5, -0.5, 0.1, 1, 0xFF00FF00},
		{-0.5, -0.5, 0.1, 1, 0xFF0000FF}
	};
#else
	vert2_t verts[3] = {
		{0, 0.5f, 0.1f, 0xFFFF0000, 0.5f, 1},
		{0.5f, -0.5f, 0.1f, 0xFF00FF00, 1, 0},
		{-0.5f, -0.5f, 0.1f, 0xFF0000FF, 0, 0},
	};
#endif
#endif

	int width = 8;
	int height = 8;

	IDirect3DTexture8 *tex = nullptr;
	r = d3dd->CreateTexture(width, height, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &tex);
	Log("%d IDirect3DDevice8::CreateTexture %p\n", r, tex);
	D3DLOCKED_RECT lockedRect;
	ZeroMemory(&lockedRect, sizeof(lockedRect));
	r = tex->LockRect(0, &lockedRect, nullptr, 0);
	Log("%d (%s) IDirect3DTexture8::LockRect %p pitch 0x%X\n", r, d3dErrorString(r).c_str(), lockedRect.pBits, lockedRect.Pitch);
	if (r == D3D_OK && lockedRect.pBits)
	{
		uint32_t *p = (uint32_t*)lockedRect.pBits;
		for (int yi = 0; yi < height; yi++)
		{
			for (int xi = 0; xi < width; xi++)
			{
				p[xi] = ((xi + yi) & 1) ? 0xFF505050 : 0xFFC0C0C0;
			}
			p += lockedRect.Pitch >> 2;
		}
		r = tex->UnlockRect(0);
		Log("%d (%s) IDirect3DTexture8::UnlockRect\n", r, d3dErrorString(r).c_str());
	}

	r = d3dd->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 100, 255), 1, 0);
	Log("%d IDirect3DDevice8::Clear\n", r);

	//render
	struct render_functor{
		IDirect3DDevice8 *d3dd;
		DWORD fvf_vert;
		IDirect3DTexture8 *tex;
		int vertCount;
		vert2_t *verts;

		void operator()()
	{
	HRESULT r = d3dd->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 100, 255), 1, 0);
	r = d3dd->BeginScene();
	if (r != D3D_OK)
		return;
#define USE_FFP 1
#if USE_FFP
	r = d3dd->SetVertexShader(fvf_vert);
#else
	r = d3dd->SetVertexShader(vertShader);
#endif
	static int frame = 0;
	frame++;
#if 0
	float ct = cos(frame * 0.01f) * 0.005;
	float st = sin(frame * 0.01f) * -0.005;
	D3DMATRIX mtxWorld = {
		ct,st,0,0,
		-st,ct,0,0,
		0,0,1,0,
		(ct - st) * -150,(st + ct) * -150,0,1
	};
#else
	float ct = cos(frame * 0.01f) * 1.5;
	float st = sin(frame * 0.01f) * 1.5;
	D3DMATRIX mtxWorld = {
		ct,st,0,0,
		-st,ct,0,0,
		0,0,1,0,
		0,0,0,1
	};
#endif
	r = d3dd->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	r = d3dd->SetRenderState(D3DRS_LIGHTING, FALSE);

	r = d3dd->SetTexture(0, tex);
	r = d3dd->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	r = d3dd->SetTransform(D3DTS_WORLD, &mtxWorld);
	r = d3dd->SetVertexShaderConstant(0, &mtxWorld, 4);
	float color[4] = { 1,0,0,1 };
	r = d3dd->SetVertexShaderConstant(4, color, 1);

#if USE_VBO
	r = d3dd->SetStreamSource(0, vbo, sizeof(vert_t));
	r = d3dd->DrawPrimitive(D3DPT_TRIANGLELIST, 0, vertCount / 3);
#else
	r = d3dd->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertCount / 3, verts, sizeof(verts[0]));
#endif
	r = d3dd->EndScene();
	}} render = {d3dd, fvf_vert, tex, vertCount, verts};

	render();
	r = d3dd->Present(nullptr, nullptr, nullptr, nullptr);
	Log("%d Present\n", r);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	//while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		render();
		r = d3dd->Present(nullptr, nullptr, nullptr, nullptr);
	}

	// shutdown
	LONG rc = d3dd->Release();
	Log("IDirect3DDevice8::Release %d\n", rc);

	rc = d3d->Release();
	Log("IDirect3D8::Release %d\n", rc);

	return 0;
}
