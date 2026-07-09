
#if (_MSC_VER > 1100)//TODO test newer versions
#include <inttypes.h>
#else
#define uint64_t unsigned __int64
#define PRIX64 "I64x" 
#define nullptr NULL
#define snprintf _snprintf
#endif

#include <stdio.h>
#include <string>
#include <vector>

#define DIRECTDRAW_VERSION 0x0700
#define DIRECT3D_VERSION 0x0700
#define INITGUID

#if(DIRECT3D_VERSION < 0x0500)
#define D3DCOLOR_MONO   1
#define D3DCOLOR_RGB    2
typedef unsigned long D3DCOLORMODEL;
#endif

#include <d3d.h>

//#ifndef DDCAPS_DX1
//#define DDCAPS_DX1 DDCAPS_DX3
//#endif

//#pragma comment(lib, "ddraw.lib")

std::string ddResultToStr(HRESULT r)
{
	if (r == DD_OK)
		return "DD_OK";

	LPSTR msg = nullptr;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		r,
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		(LPSTR)&msg,
		0, NULL);

	if (msg)
	{
		if (msg && msg[0])
		{
			auto l = strlen(msg);
			if (msg[l - 1 == '\n'])
				msg[l - 1] = 0;

			if (msg[l - 2 == '\r'])
				msg[l - 2] = 0;
		}

		std::string ret(msg);
		LocalFree(msg);
		return ret;
	}

	int sev = (r >> 31) & 1;
	int fac = (r >> 16) & 0x7FFF;
	int code = r & 0xFFFF;
	char buff[256] = { 0 };
	snprintf(buff, 255, "%d 0x%X %d", sev, fac, code);
	return std::string(buff);
}

template<typename T>
int VerboseRelease(T*& obj, const char* name)
{
	if (!obj)
		return 0;
	int c = obj->Release();
	obj = nullptr;
	//printf("%d %s::Release\n", c, name);
	return c;
}

//removed in dx5
#ifndef DDSCAPS_3D
#undef DDSCAPS_RESERVED1
#define DDSCAPS_3D 0x00000001l
#endif
//removed in dx7
#undef DDSCAPS_PRIMARYSURFACELEFT
#define DDSCAPS_PRIMARYSURFACELEFT 0x00000400l
//removed in dx7
#ifndef DDCAPS_STEREOVIEW
#define DDCAPS_STEREOVIEW 0x00040000l
#endif
//added in dx8
#ifndef DDCAPS2_PUREHAL
#define DDCAPS2_PUREHAL 0x08000000L
#endif
//removed in dx7
#ifndef DDSVCAPS_ENIGMA
#define DDSVCAPS_ENIGMA 0x00000001l
#define DDSVCAPS_FLICKER 0x00000002l
#define DDSVCAPS_REDBLUE 0x00000004l
#define DDSVCAPS_SPLIT 0x00000008l
#endif
//added in dx7
#ifndef DDSVCAPS_STEREOSEQUENTIAL
#define DDSVCAPS_STEREOSEQUENTIAL 0x00000010l
#endif
//added in dx8
#ifndef DDFXCAPS_OVERLAYDEINTERLACE
#define DDFXCAPS_OVERLAYDEINTERLACE 0x20000000l
#endif


void printDDSCaps(const char* name, DWORD c)
{
	printf("%s %X(", name, c);
#define X(name) if (c & DDSCAPS_##name) printf(#name " ");
X(3D)//DDSCAPS_RESERVED1 since DX5
X(ALPHA)
X(BACKBUFFER)
X(COMPLEX)
X(FLIP)
X(FRONTBUFFER)
X(OFFSCREENPLAIN)
X(OVERLAY)
X(PALETTE)
X(PRIMARYSURFACE)
X(PRIMARYSURFACELEFT) //DDSCAPS_RESERVED3 since DX7
X(SYSTEMMEMORY)
X(TEXTURE)
X(3DDEVICE)
X(VIDEOMEMORY)
X(VISIBLE)
X(WRITEONLY)
X(ZBUFFER)
X(OWNDC)
X(LIVEVIDEO)
X(HWCODEC)
X(MODEX)
X(MIPMAP)
X(RESERVED2)
//0x01000000l
//0x02000000l
X(ALLOCONLOAD)
//DX5
X(VIDEOPORT)
X(LOCALVIDMEM)
X(NONLOCALVIDMEM)
X(STANDARDVGAMODE)
X(OPTIMIZED)

	if (c & 0x03800000l)
		printf("Unknown flags %X", c & 0x03800000l);

#undef X
	printf(")");
}

#undef DDSCAPS2_HARDWAREDEINTERLACE
#define DDSCAPS2_HARDWAREDEINTERLACE 0x00000002L

void printDDSCaps2(const char *name, DDSCAPS2 &c)
{
	printDDSCaps(name, c.dwCaps);
	printf(" Caps2 %X(", c.dwCaps2);
#define X(name) if (c.dwCaps2 & DDSCAPS2_##name) printf(#name " ");
//0x00000001L ?
X(HARDWAREDEINTERLACE)//DDSCAPS2_RESERVED4 since DX7?
X(HINTDYNAMIC)
X(HINTSTATIC)
X(TEXTUREMANAGE)
X(RESERVED1)
X(RESERVED2)
X(OPAQUE)
X(HINTANTIALIASING)
//dx7
X(CUBEMAP)
X(CUBEMAP_POSITIVEX)
X(CUBEMAP_NEGATIVEX)
X(CUBEMAP_POSITIVEY)
X(CUBEMAP_NEGATIVEY)
X(CUBEMAP_POSITIVEZ)
X(CUBEMAP_NEGATIVEZ)
X(MIPMAPSUBLEVEL)
X(D3DTEXTUREMANAGE)
X(DONOTPERSIST)
X(STEREOSURFACELEFT)
//dx8
//0x00100000L ?
X(VOLUME)
X(NOTUSERLOCKABLE)
X(POINTS)
X(RTPATCHES)
X(NPATCHES)
X(RESERVED3)
//0x08000000L ?
X(DISCARDBACKBUFFER)
X(ENABLEALPHACHANNEL)
X(EXTENDEDFORMATPRIMARY)
X(ADDITIONALPRIMARY)

	if (c.dwCaps2 & 0x08100061l)
		printf("Unknown flags %X", c.dwCaps2 & 0x08100061l);

#undef X
	printf(") Caps3 %X(", c.dwCaps3);
#define X(name) if (c.dwCaps3 & DDSCAPS3_##name) printf(#name " ");
//MULTISAMPLE_MASK 0x0000001FL
//MULTISAMPLE_QUALITY_MASK 0x000000E0L
X(RESERVED1)
X(RESERVED2)
X(LIGHTWEIGHTMIPMAP)
X(AUTOGENMIPMAP)
X(DMAP)

	if (c.dwCaps3 & 0xFFFFE000l)
		printf("Unknown flags %X", c.dwCaps3 & 0xFFFFE000l);

#undef X
	printf(") Caps4 %X", c.dwCaps4);
}

void printPixelFormat(const char* name, DDPIXELFORMAT& pf)
{
	printf("%s size %d flags %X", name, pf.dwSize, pf.dwFlags);

	if (pf.dwFlags & DDPF_ALPHA)
	{
		printf(" ALPHA %dbits", pf.dwAlphaBitDepth);
	}

	if (pf.dwFlags & DDPF_RGB)
	{
		if (pf.dwFlags & DDPF_PALETTEINDEXED4)
			printf(" PAL4 %dbits", pf.dwRGBBitCount);
		else if (pf.dwFlags & DDPF_PALETTEINDEXED8)
			printf(" PAL8 %dbits", pf.dwRGBBitCount);
		else
			printf(" RGB%s %dbits r%X g%X b%X", (pf.dwFlags & DDPF_ALPHAPIXELS) ? "A" : "", pf.dwRGBBitCount, pf.dwRBitMask, pf.dwGBitMask, pf.dwBBitMask);

		if (pf.dwFlags & DDPF_ALPHAPIXELS)
			printf(" a%X", pf.dwRGBAlphaBitMask);
	}
	if (pf.dwFlags & DDPF_LUMINANCE)
	{
		printf(" LUMINANCE%s %dbits l%X", (pf.dwFlags & DDPF_ALPHAPIXELS) ? " ALPHA" : "", pf.dwLuminanceBitCount, pf.dwLuminanceBitMask);

		if (pf.dwFlags & DDPF_ALPHAPIXELS)
			printf(" a%X", pf.dwLuminanceAlphaBitMask);
	}

	if (pf.dwFlags & DDPF_ZBUFFER)
		printf(" ZBUFFER %dbits z%X", pf.dwZBufferBitDepth, pf.dwZBitMask);

	if (pf.dwFlags & DDPF_STENCILBUFFER)
		printf(" STENCIL %dbits s%X", pf.dwStencilBitDepth, pf.dwStencilBitMask);

	if (pf.dwFlags & DDPF_FOURCC)
		printf(" FOURCC %X %.4s", pf.dwFourCC, (const char*)&pf.dwFourCC);

	if (pf.dwFlags & DDPF_BUMPDUDV)
		printf(" BUMPDUDV %dbits u%X v%X", pf.dwBumpBitCount, pf.dwBumpDuBitMask, pf.dwBumpDvBitMask);

	DWORD handledBits = DDPF_ALPHA | DDPF_FOURCC | DDPF_RGB | DDPF_ZBUFFER | DDPF_STENCILBUFFER | DDPF_LUMINANCE | DDPF_BUMPDUDV;

	if (pf.dwFlags & (DDPF_RGB | DDPF_LUMINANCE))
		handledBits |= DDPF_ALPHAPIXELS;
	if (pf.dwFlags & DDPF_RGB)
		handledBits |= DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED8;

	if (pf.dwFlags & ~handledBits)
		printf(" Unknown flags %X", pf.dwFlags & ~handledBits);

	printf("\n");
}

//dx6 only
#ifndef DDFXCAPS_BLTTRANSFORM
#define DDFXCAPS_BLTTRANSFORM 0x00000002l
#endif
//removed in dx7
#undef DDPCAPS_INITIALIZE
#define DDPCAPS_INITIALIZE 0x00000008l

void printDDCapsFlags(const char *name, DWORD c)
{
	printf("%s %X(", name, c);
#define X(name) if (c & DDCAPS_##name) printf(#name " ");
	X(3D)
	X(ALIGNBOUNDARYDEST)
	X(ALIGNSIZEDEST)
	X(ALIGNBOUNDARYSRC)
	X(ALIGNSIZESRC)
	X(ALIGNSTRIDE)
	X(BLT)
	X(BLTQUEUE)
	X(BLTFOURCC)
	X(BLTSTRETCH)
	X(GDI)
	X(OVERLAY)
	X(OVERLAYCANTCLIP)
	X(OVERLAYFOURCC)
	X(OVERLAYSTRETCH)
	X(PALETTE)
	X(PALETTEVSYNC)
	X(READSCANLINE)
	X(STEREOVIEW)
	X(VBI)
	X(ZBLTS)
	X(ZOVERLAYS)
	X(COLORKEY)
	X(ALPHA)
	X(COLORKEYHWASSIST)
	X(NOHARDWARE)
	X(BLTCOLORFILL)
	X(BANKSWITCHED)
	X(BLTDEPTHFILL)
	X(CANCLIP)
	X(CANCLIPSTRETCHED)
	X(CANBLTSYSMEM)
#undef X
	printf(")\n");
}

void printDDCapsFlags2(const char *name, DWORD c)
{
	printf("%s %X(", name, c);
#define X(name) if (c & DDCAPS2_##name) printf(#name " ");
	X(CERTIFIED)
	X(NO2DDURING3DSCENE)
#if(DIRECTDRAW_VERSION >= 0x500)
	X(VIDEOPORT)
	X(AUTOFLIPOVERLAY)
	X(CANBOBINTERLEAVED)
	X(CANBOBNONINTERLEAVED)
	X(COLORCONTROLOVERLAY)
	X(COLORCONTROLPRIMARY)
	X(CANDROPZ16BIT)
	X(NONLOCALVIDMEM)
	X(NONLOCALVIDMEMCAPS)
	X(NOPAGELOCKREQUIRED)
	X(WIDESURFACES)
	X(CANFLIPODDEVEN)

	X(CANBOBHARDWARE)
	// CANSMOOTHINTERLEAVED ?
	// CANSMOOTHNONINTERLEAVED ?
#endif
#if(DIRECTDRAW_VERSION >= 0x600)
	X(COPYFOURCC)
	//10000
	X(PRIMARYGAMMA)
	//40000
	X(CANRENDERWINDOWED)
	X(CANCALIBRATEGAMMA)
	X(FLIPINTERVAL)
	X(FLIPNOVSYNC)
#endif
#if(DIRECTDRAW_VERSION >= 0x700)
	X(CANMANAGETEXTURE)
	X(TEXMANINNONLOCALVIDMEM)
	X(STEREO)
	X(SYSTONONLOCAL_AS_SYSTOLOCAL)
	X(PUREHAL)
	X(CANMANAGERESOURCE)
	X(DYNAMICTEXTURES)
	X(CANAUTOGENMIPMAP)
#endif
#undef X

	if (c & 0x00050000l)
		printf("Unknown flags %X", c & 0x00050000l);

	printf(")\n");
}

void printColorKeyCaps(const char *name, DWORD c)
{
	printf("%s %X(", name, c);
#define X(name) if (c & DDCKEYCAPS_##name) printf(#name " ");
	X(DESTBLT)
	X(DESTBLTCLRSPACE)
	X(DESTBLTCLRSPACEYUV)
	X(DESTBLTYUV)
	X(DESTOVERLAY)
	X(DESTOVERLAYCLRSPACE)
	X(DESTOVERLAYCLRSPACEYUV)
	X(DESTOVERLAYONEACTIVE)
	X(DESTOVERLAYYUV)
	X(SRCBLT)
	X(SRCBLTCLRSPACE)
	X(SRCBLTCLRSPACEYUV)
	X(SRCOVERLAY)
	X(SRCOVERLAYCLRSPACE)
	X(SRCOVERLAYCLRSPACEYUV)
	X(SRCOVERLAYONEACTIVE)
	X(SRCOVERLAYYUV)
	X(NOCOSTOVERLAY)
#undef X
	printf(")\n");
}

void printFXCaps(const char *name, DWORD c)
{
	printf("%s %X(", name, c);
#define X(name) if (c & DDFXCAPS_##name) printf(#name " ");
	X(BLTALPHA)//dx6+
	X(BLTTRANSFORM)//dx6
	X(OVERLAYALPHA)//dx6+
	//dx1
	X(OVERLAYARITHSTRETCHYN)
	X(BLTARITHSTRETCHYN)
	X(BLTARITHSTRETCHY)
	X(BLTMIRRORLEFTRIGHT)
	X(BLTMIRRORUPDOWN)
	X(BLTROTATION)
	X(BLTROTATION90)
	X(BLTSHRINKX)
	X(BLTSHRINKXN)
	X(BLTSHRINKY)
	X(BLTSHRINKYN)
	X(BLTSTRETCHX)
	X(BLTSTRETCHXN)
	X(BLTSTRETCHY)
	X(BLTSTRETCHYN)
	X(OVERLAYARITHSTRETCHY)
	X(OVERLAYSHRINKX)
	X(OVERLAYSHRINKXN)
	X(OVERLAYSHRINKY)
	X(OVERLAYSHRINKYN)
	X(OVERLAYSTRETCHX)
	X(OVERLAYSTRETCHXN)
	X(OVERLAYSTRETCHY)
	X(OVERLAYSTRETCHYN)
	X(OVERLAYMIRRORLEFTRIGHT)
	X(OVERLAYMIRRORUPDOWN)
	//dx8
	X(OVERLAYDEINTERLACE)
//#define DDFXCAPS_BLTFILTER              DDFXCAPS_BLTARITHSTRETCHY
//#define DDFXCAPS_OVERLAYFILTER          DDFXCAPS_OVERLAYARITHSTRETCHY
#undef X

	if (c & 0xC0000000l)
		printf("Unknown flags %X", c & 0xC0000000l);

	printf(")\n");
}

void printBitDepth(const char *name, DWORD c)
{
	printf("%s %X (", name, c);
#define X(name) if (c & DDBD_##name) printf(#name " ");
	X(1)
	X(2)
	X(4)
	X(8)
	X(16)
	X(24)
	X(32)
#define DDBD_1                  0x00004000l
#define DDBD_2                  0x00002000l
#define DDBD_4                  0x00001000l
#define DDBD_8                  0x00000800l
#define DDBD_16                 0x00000400l
#define DDBD_24                 0X00000200l
#define DDBD_32                 0x00000100l
#undef X
#define DDBD_VALID_MASK 0x00007F00L
	if (c & ~DDBD_VALID_MASK)
		printf("Unknown flags %X", c & ~DDBD_VALID_MASK);
	printf(")\n");
}

void printDD1Caps(DDCAPS_DX1& c)
{
	printDDCapsFlags(" Caps", c.dwCaps);
	printDDCapsFlags2(" Caps2", c.dwCaps2);
	printColorKeyCaps(" CKeyCaps", c.dwCKeyCaps);
	printFXCaps(" FXCaps", c.dwFXCaps);

	printf(" FXAlphaCaps %X(", c.dwFXAlphaCaps);
#define X(name) if (c.dwFXAlphaCaps & DDFXALPHACAPS_##name) printf(#name " ");
	X(BLTALPHAEDGEBLEND)
	X(BLTALPHAPIXELS)
	X(BLTALPHAPIXELSNEG)
	X(BLTALPHASURFACES)
	X(BLTALPHASURFACESNEG)
	X(OVERLAYALPHAEDGEBLEND)
	X(OVERLAYALPHAPIXELS)
	X(OVERLAYALPHAPIXELSNEG)
	X(OVERLAYALPHASURFACES)
	X(OVERLAYALPHASURFACESNEG)
#undef X

	if (c.dwFXAlphaCaps & 0xFFFFFC00l)
		printf("Unknown flags %X", c.dwFXAlphaCaps & 0xFFFFFC00l);

	printf(")\n");

	printf(" PalCaps %X(", c.dwPalCaps);
#define X(name) if (c.dwPalCaps & DDPCAPS_##name) printf(#name " ");
	X(4BIT)
	X(8BITENTRIES)
	X(8BIT)
	X(INITIALIZE)//until dx7
	X(PRIMARYSURFACE)
	X(PRIMARYSURFACELEFT)
	X(ALLOW256)
	X(VSYNC)
	X(1BIT)
	X(2BIT)
	X(ALPHA)
#undef X

	if (c.dwPalCaps & 0xFFFFF800l)
		printf("Unknown flags %X", c.dwPalCaps & 0xFFFFF800l);

	printf(")\n");

	printf(" SVCaps %X(", c.dwSVCaps);
#define X(name) if (c.dwSVCaps & DDSVCAPS_##name) printf(#name " ");
	X(ENIGMA)
	X(FLICKER)
	X(REDBLUE)
	X(SPLIT)
	X(STEREOSEQUENTIAL)
#undef X
	printf(")\n");
	
	printf(" AlphaBltConstBitDepths %X AlphaBltPixelBitDepths %X AlphaBltSurfaceBitDepths %X AlphaOverlayConstBitDepths %X AlphaOverlayPixelBitDepths %X AlphaOverlaySurfaceBitDepths\n",
		c.dwAlphaBltConstBitDepths, c.dwAlphaBltPixelBitDepths, c.dwAlphaBltSurfaceBitDepths, c.dwAlphaOverlayConstBitDepths, c.dwAlphaOverlayPixelBitDepths, c.dwAlphaOverlaySurfaceBitDepths);
	printBitDepth(" ZBufferBitDepths", c.dwZBufferBitDepths);
	printf(" VidMemTotal %u VidMemFree %u\n", c.dwVidMemTotal, c.dwVidMemFree);
	if (c.dwCaps & DDCAPS_OVERLAY)
		printf(" MaxVisibleOverlays %d CurrVisibleOverlays %d\n", c.dwMaxVisibleOverlays, c.dwCurrVisibleOverlays);

	printf(" NumFourCCCodes %d\n", c.dwNumFourCCCodes);

	if (c.dwCaps & DDCAPS_ALIGNBOUNDARYDEST)
		printf(" AlignBoundaryDest %X\n", c.dwAlignBoundaryDest);
	if (c.dwCaps & DDCAPS_ALIGNSIZEDEST)
		printf(" AlignSizeDest %X\n", c.dwAlignSizeDest);
	if (c.dwCaps & DDCAPS_ALIGNBOUNDARYSRC)
		printf(" AlignBoundarySrc %X\n", c.dwAlignBoundarySrc);
	if (c.dwCaps & DDCAPS_ALIGNSIZESRC)
		printf(" AlignSizeSrc %X\n", c.dwAlignSizeSrc);
	if (c.dwCaps & DDCAPS_ALIGNSTRIDE)
		printf(" AlignStrideAlign %X\n", c.dwAlignStrideAlign);
	
	//printf(" AlignBoundarySrc %X AlignSizeSrc %X AlignBoundaryDest %X AlignSizeDest %X AlignStrideAlign %X\n",
	//	c.dwAlignBoundarySrc, c.dwAlignSizeSrc, c.dwAlignBoundaryDest, c.dwAlignSizeDest, c.dwAlignStrideAlign);

	printf(" Rops ");
	for (int i = 0; i < DD_ROP_SPACE; i++)
		printf("%X ", c.dwRops[i]);
	printf("\n");

	printDDSCaps(" DDS Caps", c.ddsCaps.dwCaps);
	printf("\n");

	if (c.dwCaps & DDCAPS_OVERLAYSTRETCH)
		printf(" MinOverlayStretch %d MaxOverlayStretch %d\n", c.dwMinOverlayStretch, c.dwMaxOverlayStretch);

	printf(" MinLiveVideoStretch %d MaxLiveVideoStretch %d MinHwCodecStretch %d MaxHwCodecStretch %d\n",
		c.dwMinLiveVideoStretch, c.dwMaxLiveVideoStretch, c.dwMinHwCodecStretch, c.dwMaxHwCodecStretch);

	printf(" dwReserved1 %X dwReserved2 %X dwReserved3 %X\n",
		c.dwReserved1, c.dwReserved2, c.dwReserved3);
}

void printDD3Caps(DDCAPS_DX3& c)
{
	printf(" new DX3 caps:\n");
	int i = 0;
#define printBlitsCapsSet(name) \
	printDDCapsFlags(" " #name "BCaps", c.dw##name##BCaps);\
	printColorKeyCaps(" " #name "BCKeyCaps", c.dw##name##BCKeyCaps);\
	printFXCaps(" " #name "BFXCaps", c.dw##name##BFXCaps);\
	printf(" " #name "BRops "); \
	for (i = 0; i < DD_ROP_SPACE; i++) \
		printf("%X ", c.dw##name##BRops[i]); \
	printf("\n");

	printBlitsCapsSet(SV);
	printBlitsCapsSet(VS);
	printBlitsCapsSet(SS);

	//printf(" dwReserved4 %X dwReserved5 %X dwReserved6 %X\n",
	//	c.dwReserved4, c.dwReserved5, c.dwReserved6);
}

void printDD5Caps(DDCAPS_DX5& c)
{
	printf(" new DX5 caps:\n");
	printf(" MaxVideoPorts %X\n", c.dwMaxVideoPorts);
	printf(" CurrVideoPorts %X\n", c.dwCurrVideoPorts);
	printDDCapsFlags2(" SVBCaps2", c.dwSVBCaps2);
	
	int i = 0;
	printBlitsCapsSet(NLV);

	printDDCapsFlags2(" NLVBCaps2", c.dwNLVBCaps2);
}
#undef printBlitsCapsSet

void printDD6Caps(DDCAPS_DX6& c)
{
	printf(" new DX6 caps:\n");
	printDDSCaps2(" DDS Caps 2", c.ddsCaps);
	printf("\n");
}

void printD3DCmpCaps(const char* name, DWORD c)
{
	printf("%s %X (", name, c);
	if ((c & 0xFF) == 0xFF)
		printf("all");
	else
	{
#define X(name) if (c & D3DPCMPCAPS_##name) printf(#name " ");
		X(NEVER)
		X(LESS)
		X(EQUAL)
		X(LESSEQUAL)
		X(GREATER)
		X(NOTEQUAL)
		X(GREATEREQUAL)
		X(ALWAYS)
#undef X
	}
	if (c & ~0xFF)
		printf(" Unknown flags %X", c & ~0xFF);
	printf(")\n");
}

void printD3DBlendCaps(const char* name, DWORD c)
{
	printf("%s %X (", name, c);
	if ((c & 0x1FFF) == 0x1FFF)
		printf("all");
	else
	{
#define X(name) if (c & D3DPBLENDCAPS_##name) printf(#name " ");
		X(ZERO)
		X(ONE)
		X(SRCCOLOR)
		X(INVSRCCOLOR)
		X(SRCALPHA)
		X(INVSRCALPHA)
		X(DESTALPHA)
		X(INVDESTALPHA)
		X(DESTCOLOR)
		X(INVDESTCOLOR)
		X(SRCALPHASAT)
		X(BOTHSRCALPHA)
		X(BOTHINVSRCALPHA)
#undef X
	}
	if (c & ~0x1FFF)
		printf(" Unknown flags %X", c & ~0x1FFF);
	printf(")\n");
}

#ifndef D3DPRASTERCAPS_ZFOG
#define D3DPRASTERCAPS_ZFOG 0x00200000L
#endif

#ifndef D3DPTEXTURECAPS_NONPOW2CONDITIONAL
#define D3DPTEXTURECAPS_NONPOW2CONDITIONAL 0x00000100L
#endif

void printD3DPrimCaps(const char* name, const D3DPRIMCAPS& c)
{
	printf(" %s size %d\n", name, c.dwSize);
	printf("  MiscCaps %X (", c.dwMiscCaps);
#define X(name) if (c.dwMiscCaps & D3DPMISCCAPS_##name) printf(#name " ");
	X(MASKPLANES)
	X(MASKZ)
	X(LINEPATTERNREP)
	X(CONFORMANT)
	X(CULLNONE)
	X(CULLCW)
	X(CULLCCW)
#undef X
	if (c.dwMiscCaps & ~0x7F)
		printf("Unknown flags %X", c.dwMiscCaps & ~0x7F);
	printf(")\n");

	printf("  RasterCaps %X (", c.dwRasterCaps);
#define X(name) if (c.dwRasterCaps & D3DPRASTERCAPS_##name) printf(#name " ");
	X(DITHER)
	X(ROP2)
	X(XOR)
	X(PAT)
	X(ZTEST)
	X(SUBPIXEL)
	X(SUBPIXELX)
	X(FOGVERTEX)
	X(FOGTABLE)
	X(STIPPLE)
#if(DIRECT3D_VERSION >= 0x0500)
	X(ANTIALIASSORTDEPENDENT)
	X(ANTIALIASSORTINDEPENDENT)
	X(ANTIALIASEDGES)
	X(MIPMAPLODBIAS)
	X(ZBIAS)
	X(ZBUFFERLESSHSR)
	X(FOGRANGE)
	X(ANISOTROPY)
#endif /* DIRECT3D_VERSION >= 0x0500 */
#if(DIRECT3D_VERSION >= 0x0600)
	X(WBUFFER)
	X(TRANSLUCENTSORTINDEPENDENT)
	X(WFOG)
	X(ZFOG)
#endif /* DIRECT3D_VERSION >= 0x0600 */
#undef X
#if(DIRECT3D_VERSION >= 0x0600)
#define RASTERCAPS_VALID_MASK 0x3FFFFF
#elif(DIRECT3D_VERSION >= 0x0500)
#define RASTERCAPS_VALID_MASK 0x3FFFF
#else
#define RASTERCAPS_VALID_MASK 0x3FF
#endif
	if (c.dwRasterCaps & ~RASTERCAPS_VALID_MASK)
		printf("Unknown flags %X", c.dwRasterCaps & ~RASTERCAPS_VALID_MASK);
	printf(")\n");

	printD3DCmpCaps("  ZCmpCaps", c.dwZCmpCaps);
	printD3DBlendCaps("  SrcBlendCaps", c.dwSrcBlendCaps);
	printD3DBlendCaps("  DestBlendCaps", c.dwDestBlendCaps);
	printD3DCmpCaps("  AlphaCmpCaps", c.dwAlphaCmpCaps);

	printf("  ShadeCaps %X (", c.dwShadeCaps);
#define X(name) if (c.dwShadeCaps & D3DPSHADECAPS_##name) printf(#name " ");
	X(COLORFLATMONO)
	X(COLORFLATRGB)
	X(COLORGOURAUDMONO)
	X(COLORGOURAUDRGB)
	X(COLORPHONGMONO)
	X(COLORPHONGRGB)
	X(SPECULARFLATMONO)
	X(SPECULARFLATRGB)
	X(SPECULARGOURAUDMONO)
	X(SPECULARGOURAUDRGB)
	X(SPECULARPHONGMONO)
	X(SPECULARPHONGRGB)
	X(ALPHAFLATBLEND)
	X(ALPHAFLATSTIPPLED)
	X(ALPHAGOURAUDBLEND)
	X(ALPHAGOURAUDSTIPPLED)
	X(ALPHAPHONGBLEND)
	X(ALPHAPHONGSTIPPLED)
	X(FOGFLAT)
	X(FOGGOURAUD)
	X(FOGPHONG)
#undef X
	if (c.dwShadeCaps & ~0x1FFFFF)
		printf("Unknown flags %X", c.dwShadeCaps & ~0x1FFFFF);
	printf(")\n");

	printf("  TextureCaps %X (", c.dwTextureCaps);
#define X(name) if (c.dwTextureCaps & D3DPTEXTURECAPS_##name) printf(#name " ");
	X(PERSPECTIVE)
	X(POW2)
	X(ALPHA)
	X(TRANSPARENCY)
	X(BORDER)
	X(SQUAREONLY)
#if(DIRECT3D_VERSION >= 0x0600)
	X(TEXREPEATNOTSCALEDBYSIZE)
	X(ALPHAPALETTE)
	X(NONPOW2CONDITIONAL)
#endif /* DIRECT3D_VERSION >= 0x0600 */
#if(DIRECT3D_VERSION >= 0x0700)
	//0x00000200L
	X(PROJECTED)
	X(CUBEMAP)
	X(COLORKEYBLEND)
#endif
#undef X
#if(DIRECT3D_VERSION >= 0x0700)
#define TEXTURECAPS_VALID_MASK 0x1DFF
#elif(DIRECT3D_VERSION >= 0x0600)
#define TEXTURECAPS_VALID_MASK 0x1FF
#else
#define TEXTURECAPS_VALID_MASK 0x3F
#endif
	if (c.dwTextureCaps & ~TEXTURECAPS_VALID_MASK)
		printf("Unknown flags %X", c.dwTextureCaps & ~TEXTURECAPS_VALID_MASK);
	printf(")\n");

	printf("  TextureFilterCaps %X (", c.dwTextureFilterCaps);
#define X(name) if (c.dwTextureFilterCaps & D3DPTFILTERCAPS_##name) printf(#name " ");
	X(NEAREST)
	X(LINEAR)
	X(MIPNEAREST)
	X(MIPLINEAR)
	X(LINEARMIPNEAREST)
	X(LINEARMIPLINEAR)
#if(DIRECT3D_VERSION >= 0x0600)
//0x00000040L
//0x00000080L
	X(MINFPOINT)
	X(MINFLINEAR)
	X(MINFANISOTROPIC)
//0x00000800L
	X(MIPFPOINT)
	X(MIPFLINEAR)
//0x00040000L-0x00800000L
	X(MAGFPOINT)
	X(MAGFLINEAR)
	X(MAGFANISOTROPIC)
	X(MAGFAFLATCUBIC)
	X(MAGFGAUSSIANCUBIC)
#endif /* DIRECT3D_VERSION >= 0x0600 */
#undef X
#if(DIRECT3D_VERSION >= 0x0600)
#define TFILTERCAPS_VALID_MASK 0x1F03073F
#else
#define TFILTERCAPS_VALID_MASK 0x3F
#endif
	if (c.dwTextureFilterCaps & ~TFILTERCAPS_VALID_MASK)
		printf("Unknown flags %X", c.dwTextureFilterCaps & ~TFILTERCAPS_VALID_MASK);
	printf(")\n");

	printf("  TextureBlendCaps %X (", c.dwTextureBlendCaps);
#define X(name) if (c.dwTextureBlendCaps & D3DPTBLENDCAPS_##name) printf(#name " ");
	X(DECAL)
	X(MODULATE)
	X(DECALALPHA)
	X(MODULATEALPHA)
	X(DECALMASK)
	X(MODULATEMASK)
	X(COPY)
#if(DIRECT3D_VERSION >= 0x0500)
	X(ADD)
#endif /* DIRECT3D_VERSION >= 0x0500 */
#undef X
#if(DIRECT3D_VERSION >= 0x0500)
#define TBLENDCAPS_VALID_MASK 0xFF
#else
#define TBLENDCAPS_VALID_MASK 0x7F
#endif
	if (c.dwTextureBlendCaps & ~TBLENDCAPS_VALID_MASK)
		printf("Unknown flags %X", c.dwTextureBlendCaps & ~TBLENDCAPS_VALID_MASK);
	printf(")\n");

	printf("  TextureAddressCaps %X (", c.dwTextureAddressCaps);
#define X(name) if (c.dwTextureAddressCaps & D3DPTADDRESSCAPS_##name) printf(#name " ");
	X(WRAP)
	X(MIRROR)
	X(CLAMP)
#if(DIRECT3D_VERSION >= 0x0500)
	X(BORDER)
	X(INDEPENDENTUV)
#endif /* DIRECT3D_VERSION >= 0x0500 */
#undef X
#if(DIRECT3D_VERSION >= 0x0500)
#define TADDRESSCAPS_VALID_MASK 0x1F
#else
#define TADDRESSCAPS_VALID_MASK 0x7
#endif
	if (c.dwTextureAddressCaps & ~TADDRESSCAPS_VALID_MASK)
		printf("Unknown flags %X", c.dwTextureAddressCaps & ~TADDRESSCAPS_VALID_MASK);
	printf(")\n");

	printf("  Stipple %dx%d\n", c.dwStippleWidth, c.dwStippleHeight);
}

#ifndef D3DDEVCAPS_DRAWPRIMITIVES2EX
#define D3DDEVCAPS_DRAWPRIMITIVES2EX 0x00008000L
#endif

void printD3DDevCaps(DWORD c)
{
	printf(" DevCaps %X (", c);
#define X(name) if (c & D3DDEVCAPS_##name) printf(#name " ");
	X(FLOATTLVERTEX)
	X(SORTINCREASINGZ)
	X(SORTDECREASINGZ)
	X(SORTEXACT)
	X(EXECUTESYSTEMMEMORY)
	X(EXECUTEVIDEOMEMORY)
	X(TLVERTEXSYSTEMMEMORY)
	X(TLVERTEXVIDEOMEMORY)
	X(TEXTURESYSTEMMEMORY)
	X(TEXTUREVIDEOMEMORY)
#if(DIRECT3D_VERSION >= 0x0500)
	X(DRAWPRIMTLVERTEX)
	X(CANRENDERAFTERFLIP)
	X(TEXTURENONLOCALVIDMEM)
#endif /* DIRECT3D_VERSION >= 0x0500 */
#if(DIRECT3D_VERSION >= 0x0600)
	X(DRAWPRIMITIVES2)
	X(SEPARATETEXTUREMEMORIES)
	X(DRAWPRIMITIVES2EX)
#endif /* DIRECT3D_VERSION >= 0x0600 */
#if(DIRECT3D_VERSION >= 0x0700)
	X(HWTRANSFORMANDLIGHT)
	X(CANBLTSYSTONONLOCAL)
	//0x00040000L
	X(HWRASTERIZATION)
#endif
#undef X
#if(DIRECT3D_VERSION >= 0x0700)
#define DEVCAPS_VALID_MASK 0xBFFFF
#elif(DIRECT3D_VERSION >= 0x0600)
#define DEVCAPS_VALID_MASK 0xFFFF
#elif(DIRECT3D_VERSION >= 0x0500)
#define DEVCAPS_VALID_MASK 0x1FFF
#else
#define DEVCAPS_VALID_MASK 0x3FF
#endif
	if (c & ~DEVCAPS_VALID_MASK)
		printf("Unknown flags %X", c & ~DEVCAPS_VALID_MASK);
	printf(")\n");
}

// removed in dx7
#ifndef D3DLIGHTCAPS_PARALLELPOINT
#define D3DLIGHTCAPS_PARALLELPOINT 0x00000008L
#endif
// removed in dx5
#ifndef D3DLIGHTCAPS_GLSPOT
#define D3DLIGHTCAPS_GLSPOT 0x00000010L
#endif

void printD3DDCaps(const char* name, const D3DDEVICEDESC &c)
{
	printf("%s caps: flags %X\n", name, c.dwFlags);
	if (c.dwFlags & D3DDD_COLORMODEL)
		printf(" ColorModel %X\n", c.dcmColorModel);
	if (c.dwFlags * D3DDD_DEVCAPS)
	{
		printD3DDevCaps(c.dwDevCaps);
	}
	if (c.dwFlags & D3DDD_TRANSFORMCAPS)
	{
		printf(" TransformCaps size %d caps %X (", c.dtcTransformCaps.dwSize, c.dtcTransformCaps.dwCaps);
		if (c.dtcTransformCaps.dwCaps & D3DTRANSFORMCAPS_CLIP)
			printf("CLIP ");
		if (c.dtcTransformCaps.dwCaps & ~0x00000001L)
			printf("Unknown flags %X", c.dtcTransformCaps.dwCaps & ~0x00000001L);
		printf(")\n");
	}
	if (c.dwFlags & D3DDD_LIGHTINGCAPS)
	{
		printf(" LightingCaps size %d model %X lights %d caps %X (", c.dlcLightingCaps.dwSize, c.dlcLightingCaps.dwLightingModel, c.dlcLightingCaps.dwNumLights, c.dlcLightingCaps.dwCaps);
#define X(name) if (c.dlcLightingCaps.dwCaps & D3DLIGHTCAPS_##name) printf(#name " ");
		X(POINT)
		X(SPOT)
		X(DIRECTIONAL)
		X(PARALLELPOINT)
		X(GLSPOT)
#undef X
		if (c.dlcLightingCaps.dwCaps & ~0x1F)
			printf("Unknown flags %X", c.dlcLightingCaps.dwCaps & ~0x1F);
		printf(")\n");
	}
	if (c.dwFlags & D3DDD_BCLIPPING)
		printf(" bClipping %d\n", c.bClipping);
	if (c.dwFlags & D3DDD_LINECAPS)
		printD3DPrimCaps("LineCaps", c.dpcLineCaps);
	if (c.dwFlags & D3DDD_TRICAPS)
		printD3DPrimCaps("TriCaps", c.dpcTriCaps);
	if (c.dwFlags & D3DDD_DEVICERENDERBITDEPTH)
		printBitDepth(" DeviceRenderBitDepth", c.dwDeviceRenderBitDepth);
	if (c.dwFlags & D3DDD_DEVICEZBUFFERBITDEPTH)
		printBitDepth(" DeviceZBufferBitDepth", c.dwDeviceZBufferBitDepth);
	if (c.dwFlags & D3DDD_MAXBUFFERSIZE)
		printf(" MaxBufferSize %d\n", c.dwMaxBufferSize);
	if (c.dwFlags & D3DDD_MAXVERTEXCOUNT)
		printf(" MaxVertexCount %d\n", c.dwMaxVertexCount);
#if(DIRECT3D_VERSION >= 0x0500)
	printf(" Texture min %dx%d max %dx%d Stipple min %dx%d max %dx%d\n",
		c.dwMinTextureWidth, c.dwMinTextureHeight, c.dwMaxTextureWidth, c.dwMaxTextureHeight,
		c.dwMinStippleWidth, c.dwMinStippleHeight, c.dwMaxStippleWidth, c.dwMaxStippleHeight);
#endif /* DIRECT3D_VERSION >= 0x0500 */
}

void printD3DDCaps7(const D3DDEVICEDESC7 &c)
{
	uint64_t guid[2] = { 0 };
	memcpy(guid, &c.deviceGUID, sizeof(GUID));
	printf(" deviceGUID %.16" PRIX64 "%.16" PRIX64 "\n", guid[0], guid[1]);

	printD3DDevCaps(c.dwDevCaps);
	printD3DPrimCaps("LineCaps", c.dpcLineCaps);
	printD3DPrimCaps("TriCaps", c.dpcTriCaps);

	printBitDepth(" DeviceRenderBitDepth", c.dwDeviceRenderBitDepth);
	printBitDepth(" DeviceZBufferBitDepth", c.dwDeviceZBufferBitDepth);
	printf(" Texture min %dx%d max %dx%d MaxRepeat %d MaxAspectRatio %d MaxAnisotropy %d\n",
		c.dwMinTextureWidth, c.dwMinTextureHeight, c.dwMaxTextureWidth, c.dwMaxTextureHeight,
		c.dwMaxTextureRepeat, c.dwMaxTextureAspectRatio, c.dwMaxAnisotropy);
	printf(" GuardBand l %g t %g r %g b %g\n",
		c.dvGuardBandLeft, c.dvGuardBandTop, c.dvGuardBandRight, c.dvGuardBandBottom);
	printf(" ExtentsAdjust %g\n", c.dvExtentsAdjust);

	printf(" StencilCaps %X (", c.dwStencilCaps);
#define X(name) if (c.dwStencilCaps & D3DSTENCILCAPS_##name) printf(#name " ");
	X(KEEP)
	X(ZERO)
	X(REPLACE)
	X(INCRSAT)
	X(DECRSAT)
	X(INVERT)
	X(INCR)
	X(DECR)
#undef X
#define D3DSTENCILCAPS_VALID_MASK 0x000000FFL
	if (c.dwStencilCaps & ~D3DSTENCILCAPS_VALID_MASK)
		printf("Unknown flags %X", c.dwStencilCaps & ~D3DSTENCILCAPS_VALID_MASK);
	printf(")\n");

	printf(" FVFCaps %X texCoords %d (", c.dwFVFCaps, c.dwFVFCaps & D3DFVFCAPS_TEXCOORDCOUNTMASK);
#define X(name) if (c.dwFVFCaps & D3DFVFCAPS_##name) printf(#name " ");
	X(DONOTSTRIPELEMENTS)
#undef X
#define D3DFVFCAPS_VALID_MASK 0x0008FFFFL
	if (c.dwFVFCaps & ~D3DFVFCAPS_VALID_MASK)
		printf("Unknown flags %X", c.dwFVFCaps & ~D3DFVFCAPS_VALID_MASK);
	printf(")\n");

	printf(" TextureOpCaps %X (", c.dwTextureOpCaps);
#define X(name) if (c.dwTextureOpCaps & D3DTEXOPCAPS_##name) printf(#name " ");
	X(DISABLE)
	X(SELECTARG1)
	X(SELECTARG2)
	X(MODULATE)
	X(MODULATE2X)
	X(MODULATE4X)
	X(ADD)
	X(ADDSIGNED)
	X(ADDSIGNED2X)
	X(SUBTRACT)
	X(ADDSMOOTH)
	X(BLENDDIFFUSEALPHA)
	X(BLENDTEXTUREALPHA)
	X(BLENDFACTORALPHA)
	X(BLENDTEXTUREALPHAPM)
	X(BLENDCURRENTALPHA)
	X(PREMODULATE)
	X(MODULATEALPHA_ADDCOLOR)
	X(MODULATECOLOR_ADDALPHA)
	X(MODULATEINVALPHA_ADDCOLOR)
	X(MODULATEINVCOLOR_ADDALPHA)
	X(BUMPENVMAP)
	X(BUMPENVMAPLUMINANCE)
	X(DOTPRODUCT3)
#undef X
#define D3DTEXOPCAPS_VALID_MASK 0x00FFFFFFL
	if (c.dwTextureOpCaps & ~D3DTEXOPCAPS_VALID_MASK)
		printf("Unknown flags %X", c.dwTextureOpCaps & ~D3DTEXOPCAPS_VALID_MASK);
	printf(")\n");

	printf(" MaxTextureBlendStages %d MaxSimultaneousTextures %d\n",
		c.wMaxTextureBlendStages, c.wMaxSimultaneousTextures);
	printf(" MaxActiveLights %d\n", c.dwMaxActiveLights);
	printf(" MaxVertexW %g\n", c.dvMaxVertexW);

	printf(" MaxUserClipPlanes %d MaxVertexBlendMatrices %d\n",
		c.wMaxUserClipPlanes, c.wMaxVertexBlendMatrices);

	printf(" VertexProcessingCaps %X (", c.dwVertexProcessingCaps);
#define X(name) if (c.dwVertexProcessingCaps & D3DVTXPCAPS_##name) printf(#name " ");
	X(TEXGEN)
	X(MATERIALSOURCE7)
	X(VERTEXFOG)
	X(DIRECTIONALLIGHTS)
	X(POSITIONALLIGHTS)
	X(LOCALVIEWER)
#undef X
#define D3DVTXPCAPS_VALID_MASK 0x3F
	if (c.dwVertexProcessingCaps & ~D3DVTXPCAPS_VALID_MASK)
		printf("Unknown flags %X", c.dwVertexProcessingCaps & ~D3DVTXPCAPS_VALID_MASK);
	printf(")\n");

	printf(" dwReserved1 %X dwReserved2 %X dwReserved3 %X dwReserved4 %X\n",
		c.dwReserved1, c.dwReserved2, c.dwReserved3, c.dwReserved4);
}

struct enumModesCtx_t
{
	enumModesCtx_t() : count(0){}
	//std::vector<DDSURFACEDESC> modes;
	//msvc5 just can't :(
	std::vector<char> modes;
	int count;

	void print()
	{
		printf(" %d modes\n", count);
		for (int i = 0; i < count; i++)
		{
			//DDSURFACEDESC &d = modes[i];
			DDSURFACEDESC &d = *(DDSURFACEDESC*)&modes[i * sizeof(DDSURFACEDESC)];
			printf(" %2d: %dx%d %dHz %dbit", i, d.dwWidth, d.dwHeight, d.dwRefreshRate, d.ddpfPixelFormat.dwRGBBitCount);
			printf(" flags %X pitch %d ", d.dwFlags, d.lPitch);
			printPixelFormat("pf", d.ddpfPixelFormat);
		}
	}

	void print2()
	{
		printf(" %d modes\n", count);
		for (int i = 0; i < count; i++)
		{
			//DDSURFACEDESC &d = modes[i];
			DDSURFACEDESC2 &d = *(DDSURFACEDESC2*)&modes[i * sizeof(DDSURFACEDESC2)];
			printf(" %2d: %dx%d %dHz %dbit", i, d.dwWidth, d.dwHeight, d.dwRefreshRate, d.ddpfPixelFormat.dwRGBBitCount);
			printf(" flags %X pitch %d ", d.dwFlags, d.lPitch);
			printPixelFormat("pf", d.ddpfPixelFormat);
		}
	}
};

HRESULT CALLBACK enumModes (LPDDSURFACEDESC lpDDSDesc, LPVOID lpContext)
{
	enumModesCtx_t &ctx = *(enumModesCtx_t*)lpContext;
	//ctx.modes.push_back(*lpDDSDesc);
	ctx.modes.resize(ctx.modes.size() + sizeof(DDSURFACEDESC));
	memcpy(&ctx.modes[ctx.count * sizeof(DDSURFACEDESC)], lpDDSDesc, sizeof(DDSURFACEDESC));
	ctx.count++;
	return DDENUMRET_OK;
}

HRESULT CALLBACK enumModes2 (LPDDSURFACEDESC2 lpDDSDesc, LPVOID lpContext)
{
	enumModesCtx_t &ctx = *(enumModesCtx_t*)lpContext;
	//ctx.modes.push_back(*lpDDSDesc);
	ctx.modes.resize(ctx.modes.size() + sizeof(DDSURFACEDESC2));
	memcpy(&ctx.modes[ctx.count * sizeof(DDSURFACEDESC2)], lpDDSDesc, sizeof(DDSURFACEDESC2));
	ctx.count++;
	return DDENUMRET_OK;
}

void printModes(IDirectDraw *pdd, IDirectDraw2 *pdd2, IDirectDraw4 *pdd4, IDirectDraw7 *pdd7)
{
	HRESULT r;

	printf("DirectDraw display modes:\n");
	printf("DX1: (REFRESHRATES)\n");

	enumModesCtx_t modes1;
	r = pdd->EnumDisplayModes(DDEDM_REFRESHRATES, nullptr, &modes1, enumModes);
	if (r != DD_OK)
		printf("%d (%s) IDirectDraw::EnumDisplayModes count %d\n", r, ddResultToStr(r).c_str(), modes1.count);
	modes1.print();

	if (pdd2)
	{
		printf("DX2: (REFRESHRATES)\n");
		enumModesCtx_t modes2;
		r = pdd2->EnumDisplayModes(DDEDM_REFRESHRATES, nullptr, &modes2, enumModes);
		if (r != DD_OK)
			printf("%d (%s) DD2::EnumDisplayModes count %d\n", r, ddResultToStr(r).c_str(), modes2.count);
		if (modes2.modes == modes1.modes)
		{
			printf(" same as DX1\n");
		}
		else
			modes2.print();
	}

	enumModesCtx_t modes6;
	if (pdd4)
	{
		printf("DX6: (REFRESHRATES)\n");
		r = pdd4->EnumDisplayModes(DDEDM_REFRESHRATES, nullptr, &modes6, enumModes2);
		if (r != DD_OK)
			printf("%d (%s) DD4::EnumDisplayModes count %d\n", r, ddResultToStr(r).c_str(), modes6.count);
		modes6.print2();
	}

	if (pdd7)
	{
		printf("DX7: (REFRESHRATES)\n");
		enumModesCtx_t modes7;
		r = pdd7->EnumDisplayModes(DDEDM_REFRESHRATES, nullptr, &modes7, enumModes2);
		if (r != DD_OK)
			printf("%d (%s) DD7::EnumDisplayModes count %d\n", r, ddResultToStr(r).c_str(), modes7.count);
		if (modes7.modes == modes6.modes)
		{
			printf(" same as DX6\n");
		}
		else
			modes7.print();
	}

}

HRESULT CALLBACK enumSurf(LPDIRECTDRAWSURFACE lpDDSurface, LPDDSURFACEDESC lpDesc, LPVOID lpContext)
{
	printf(" %d: surf %p desc %p\n", *(int*)lpContext, lpDDSurface, lpDesc);
	if (lpDesc)
	{
		printf("  desc: flags %X caps %X size %dx%d pitch %d\n", lpDesc->dwFlags, lpDesc->ddsCaps.dwCaps, lpDesc->dwWidth, lpDesc->dwHeight, lpDesc->lPitch);
		printPixelFormat("  pf", lpDesc->ddpfPixelFormat);

	}
	if (lpDDSurface)
		VerboseRelease(lpDDSurface, "IDirectDrawSurface");
	(*(int*)lpContext)++;
	return DDENUMRET_OK;
}

void printSurfaces(IDirectDraw *pdd, IDirectDraw2 *pdd2, IDirectDraw4 *pdd4, IDirectDraw7 *pdd7)
{
	HRESULT r;
	int count = 0;

#if 1
	r = pdd->EnumSurfaces(DDENUMSURFACES_DOESEXIST | DDENUMSURFACES_ALL, nullptr, &count, enumSurf);
	//finds nothing
	printf("%d (%s) IDirectDraw::EnumSurfaces(DDENUMSURFACES_DOESEXIST | DDENUMSURFACES_ALL) count %d\n", r, ddResultToStr(r).c_str(), count);

	count = 0;
	DDSURFACEDESC ddsd = { 0 };
	ddsd.dwSize = sizeof(ddsd);
	r = pdd->EnumSurfaces(DDENUMSURFACES_CANBECREATED | DDENUMSURFACES_MATCH, &ddsd, &count, enumSurf);
	//finds all display modes but without refresh rates
	printf("%d (%s) IDirectDraw::EnumSurfaces(DDENUMSURFACES_CANBECREATED | DDENUMSURFACES_MATCH) count %d\n", r, ddResultToStr(r).c_str(), count);
#endif
#if 0
	if (pdd2)
	{
		count = 0;
		r = pdd2->EnumSurfaces(DDENUMSURFACES_CANBECREATED | DDENUMSURFACES_MATCH, &ddsd, &count, enumSurf);
		printf("%d (%s) DD2::EnumSurfaces(DDENUMSURFACES_CANBECREATED | DDENUMSURFACES_MATCH) count %d\n", r, ddResultToStr(r).c_str(), count);
	}

	if (pdd4)
	{
		auto enumSurf4 = [](LPDIRECTDRAWSURFACE4 lpDDSurface, LPDDSURFACEDESC2 lpDesc, LPVOID lpContext) ->HRESULT
			{
				printf(" %d: surf %p desc %p\n", *(int*)lpContext, lpDDSurface, lpDesc);
				if (lpDesc)
				{
					printf("  desc: flags %X caps %X size %dx%d pitch %d\n", lpDesc->dwFlags, lpDesc->ddsCaps.dwCaps, lpDesc->dwWidth, lpDesc->dwHeight, lpDesc->lPitch);
					printf("  pf(flags %X rgbBits %d)\n", lpDesc->ddpfPixelFormat.dwFlags, lpDesc->ddpfPixelFormat.dwRGBBitCount);

				}
				if (lpDDSurface)
					VerboseRelease(lpDDSurface, "IDirectDrawSurface");
				(*(int*)lpContext)++;
				return DDENUMRET_OK;
			};

		count = 0;
		r = pdd4->EnumSurfaces(DDENUMSURFACES_DOESEXIST | DDENUMSURFACES_ALL, nullptr, &count, enumSurf4);
		printf("%d (%s) DD4::EnumSurfaces(DDENUMSURFACES_DOESEXIST | DDENUMSURFACES_ALL) count %d\n", r, ddResultToStr(r).c_str(), count);


		count = 0;
		DDSURFACEDESC2 ddsd2{ 0 };
		ddsd2.dwSize = sizeof(ddsd2);
		r = pdd4->EnumSurfaces(DDENUMSURFACES_CANBECREATED | DDENUMSURFACES_MATCH, &ddsd2, &count, enumSurf4);
		printf("%d (%s) DD4::EnumSurfaces(DDENUMSURFACES_CANBECREATED | DDENUMSURFACES_MATCH) count %d\n", r, ddResultToStr(r).c_str(), count);
	}
#endif
}

HRESULT CALLBACK enumDevicesPrint(GUID FAR* lpGUID, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC descHW, LPD3DDEVICEDESC descHEL, LPVOID lpContext)
{
	uint64_t guid[2] = { 0 };
	if (lpGUID)
		memcpy(guid, lpGUID, sizeof(GUID));

	printf(" guid %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDeviceName, lpDeviceDescription);

	(*(int*)lpContext)++;
	return DDENUMRET_OK;
}

HRESULT CALLBACK enumDevicesPrint7(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 descHW, LPVOID lpContext)
{
	uint64_t guid[2] = { 0 };
	if (descHW)
		memcpy(guid, &descHW->deviceGUID, sizeof(GUID));

	printf(" guid %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDeviceName, lpDeviceDescription);

	(*(int*)lpContext)++;
	return DDENUMRET_OK;
}

void printD3DDevices(IDirect3D* pd3d, IDirect3D2* pd3d2, IDirect3D3* pd3d3, IDirect3D7* pd3d7)
{
	HRESULT r;
	int count = 0;

	printf("D3D Devices:\n");
	if (pd3d)
	{
		printf("DX2:\n");
		count = 0;
		r = pd3d->EnumDevices(enumDevicesPrint, &count);
		if (r != DD_OK)
			printf("%d (%s) D3D::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);
	}
#if(DIRECT3D_VERSION >= 0x0500)
	if (pd3d2)
	{
		printf("DX5:\n");
		count = 0;
		r = pd3d2->EnumDevices(enumDevicesPrint, &count);
		if (r != DD_OK)
			printf("%d (%s) D3D2::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);
	}
#endif
#if(DIRECT3D_VERSION >= 0x0600)
	if (pd3d3)
	{
		printf("DX6:\n");
		count = 0;
		r = pd3d3->EnumDevices(enumDevicesPrint, &count);
		if (r != DD_OK)
			printf("%d (%s) D3D3::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);
	}
#endif
#if(DIRECT3D_VERSION >= 0x0700)
	if (pd3d7)
	{
		printf("DX7:\n");
		count = 0;
		r = pd3d7->EnumDevices(enumDevicesPrint7, &count);
		if (r != DD_OK)
			printf("%d (%s) D3D7::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);
	}
#endif
	printf("\n");
}

struct enum1ctx
{
	int count;
	IDirectDraw* pdd;
	IDirect3D2* pd3d2;
};

HRESULT CALLBACK enumDDSD(LPDDSURFACEDESC lpDdsd, LPVOID lpContext)
{
	printf("  size %d flags %X", lpDdsd->dwSize, lpDdsd->dwFlags);
	if (lpDdsd->dwFlags & DDSD_CAPS)
		printDDSCaps(" caps", lpDdsd->ddsCaps.dwCaps);
	if (lpDdsd->dwFlags & DDSD_PIXELFORMAT)
		printPixelFormat(" pf", lpDdsd->ddpfPixelFormat);
	(*(int*)lpContext)++;
	return DDENUMRET_OK;
}

HRESULT CALLBACK enumDevicesFormats1(GUID FAR* lpGUID, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC descHW, LPD3DDEVICEDESC descHEL, LPVOID lpContext)
{
	enum1ctx& ctx = *(enum1ctx*)lpContext;
	uint64_t guid[2] = { 0 };
	if (lpGUID)
		memcpy(guid, lpGUID, sizeof(GUID));

	printf(" guid %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDeviceName, lpDeviceDescription);

	DDSURFACEDESC ddsd = {0};
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE;
	ddsd.dwWidth = 256;
	ddsd.dwHeight = 256;
	IDirectDrawSurface* surf = nullptr;
	HRESULT r = ctx.pdd->CreateSurface(&ddsd, &surf, nullptr);
	if (!surf || r != DD_OK)
		printf("%d (%s) IDirectDraw::CreateSurface(3DDEVICE) %p\n", r, ddResultToStr(r).c_str(), surf);
	//else
	//	printf(" Created surface (3DDEVICE)\n");
	
	if (surf)
	{
		if (!ctx.pd3d2)
		{
			IDirect3DDevice* pd3dd = nullptr;
			r = surf->QueryInterface(*lpGUID, (void**)&pd3dd);
			if (!pd3dd || r != DD_OK)
				printf("%d (%s) IDirectDrawSurface::QueryInterface(IDirect3DDevice) %p\n", r, ddResultToStr(r).c_str(), pd3dd);
			//else
			//	printf(" QueryInterface(D3D Device)\n");

			if (pd3dd)
			{
				int fmtCount = 0;
				r = pd3dd->EnumTextureFormats(enumDDSD, &fmtCount);
				if (r != DD_OK)
					printf("%d (%s) D3DD::EnumTextureFormats count %d\n", r, ddResultToStr(r).c_str(), fmtCount);

				VerboseRelease(pd3dd, "IDirect3DDevice");
			}
		}
		else
		{
			IDirect3DDevice2* pd3dd2 = nullptr;
			r = ctx.pd3d2->CreateDevice(*lpGUID, surf, &pd3dd2);
			if (!pd3dd2 || r != DD_OK)
				printf("%d (%s) D3D2::CreateDevice %p\n", r, ddResultToStr(r).c_str(), pd3dd2);
			//else
			//	printf(" Created D3D Device\n");

			if (pd3dd2)
			{
				int fmtCount = 0;
				r = pd3dd2->EnumTextureFormats(enumDDSD, &fmtCount);
				if (r != DD_OK)
					printf("%d (%s) D3DD2::EnumTextureFormats count %d\n", r, ddResultToStr(r).c_str(), fmtCount);

				VerboseRelease(pd3dd2, "IDirect3DDevice2");
			}
		}

		VerboseRelease(surf, "IDirectDrawSurface");
	}

	ctx.count++;
	return DDENUMRET_OK;
};

struct enum3ctx
{
	int count;
	IDirectDraw4* pdd4;
	IDirect3D3* pd3d3;
};

HRESULT CALLBACK enumFmt(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext)
{
	printPixelFormat("  pf", *lpDDPixFmt);
	(*(int*)lpContext)++;
	return DDENUMRET_OK;
};

HRESULT CALLBACK enumDevicesFormats3(GUID FAR* lpGUID, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC descHW, LPD3DDEVICEDESC descHEL, LPVOID lpContext)
{
	enum3ctx& ctx = *(enum3ctx*)lpContext;
	uint64_t guid[2] = { 0 };
	if (lpGUID)
		memcpy(guid, lpGUID, sizeof(GUID));

	printf(" guid %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDeviceName, lpDeviceDescription);

	DDSURFACEDESC2 ddsd = {0};
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE;
	ddsd.dwWidth = 256;
	ddsd.dwHeight = 256;
	IDirectDrawSurface4* surf = nullptr;
	HRESULT r = ctx.pdd4->CreateSurface(&ddsd, &surf, nullptr);
	if (!surf || r != DD_OK)
		printf("%d (%s) IDirectDraw4::CreateSurface(3DDEVICE) %p\n", r, ddResultToStr(r).c_str(), surf);
	//else
	//	printf(" Created surface (3DDEVICE)\n");

	if (surf)
	{
		IDirect3DDevice3* pd3dd = nullptr;
		r = ctx.pd3d3->CreateDevice(*lpGUID, surf, &pd3dd, nullptr);
		if (!pd3dd || r != DD_OK)
			printf("%d (%s) D3D3::CreateDevice %p\n", r, ddResultToStr(r).c_str(), pd3dd);
		//else
		//	printf(" Created D3D Device\n");

		if (pd3dd)
		{
			int fmtCount = 0;
			r = pd3dd->EnumTextureFormats(enumFmt, &fmtCount);
			if (r != DD_OK)
				printf("%d (%s) D3DD3::EnumTextureFormats count %d\n", r, ddResultToStr(r).c_str(), fmtCount);

			VerboseRelease(pd3dd, "IDirect3DDevice3");
		}

		VerboseRelease(surf, "IDirectDrawSurface4");
	}

	printf(" EnumZBufferFormats:\n");
	int fmtCount = 0;
	r = ctx.pd3d3->EnumZBufferFormats(*lpGUID, enumFmt, &fmtCount);
	if (r != DD_OK)
		printf("%d (%s) D3D3::EnumZBufferFormats count %d\n", r, ddResultToStr(r).c_str(), fmtCount);

	ctx.count++;
	return DDENUMRET_OK;
}

struct enum7ctx
{
	int count;
	IDirectDraw7* pdd7;
	IDirect3D7* pd3d7;
};

HRESULT CALLBACK enumDevicesFormats7(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 desc, LPVOID lpContext)
{
	enum7ctx& ctx = *(enum7ctx*)lpContext;

	uint64_t guid[2] = { 0 };
	if (desc)
		memcpy(guid, &desc->deviceGUID, sizeof(GUID));

	printf(" guid %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDeviceName, lpDeviceDescription);

	DDSURFACEDESC2 ddsd = {0};
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE;//DDSCAPS_OFFSCREENPLAIN
	ddsd.dwWidth = 256;
	ddsd.dwHeight = 256;
	IDirectDrawSurface7* surf = nullptr;
	HRESULT r = ctx.pdd7->CreateSurface(&ddsd, &surf, nullptr);
	if (!surf || r != DD_OK)
		printf("%d (%s) IDirectDraw7::CreateSurface(3DDEVICE) %p\n", r, ddResultToStr(r).c_str(), surf);

	if (surf)
	{
		IDirect3DDevice7* pd3dd7 = nullptr;
		r = ctx.pd3d7->CreateDevice(desc->deviceGUID, surf, &pd3dd7);
		if (!pd3dd7 || r != DD_OK)
			printf("%d (%s) D3D7::CreateDevice %p\n", r, ddResultToStr(r).c_str(), pd3dd7);

		if (pd3dd7)
		{
			int fmtCount = 0;
			r = pd3dd7->EnumTextureFormats(enumFmt, &fmtCount);
			if (r != DD_OK)
				printf("%d (%s) D3DD7::EnumTextureFormats count %d\n", r, ddResultToStr(r).c_str(), fmtCount);

			VerboseRelease(pd3dd7, "IDirect3DDevice7");
		}

		VerboseRelease(surf, "IDirectDrawSurface7");
	}

	printf(" EnumZBufferFormats:\n");
	int fmtCount = 0;
	r = ctx.pd3d7->EnumZBufferFormats(desc->deviceGUID, enumFmt, &fmtCount);
	if (r != DD_OK)
		printf("%d (%s) D3D7::EnumZBufferFormats count %d\n", r, ddResultToStr(r).c_str(), fmtCount);

	ctx.count++;
	return DDENUMRET_OK;
}

void printD3DFormats(IDirectDraw *pdd, IDirectDraw4 *pdd4, IDirectDraw7 *pdd7, IDirect3D* pd3d, IDirect3D2* pd3d2, IDirect3D3* pd3d3, IDirect3D7* pd3d7)
{
	printf("D3D Device formats:\n");
	HRESULT r;
	if (pd3d)
	{
		printf("DX2:\n");

		r = pdd->SetCooperativeLevel(nullptr, DDSCL_NORMAL);
		if (r != DD_OK)
			printf("%d (%s) IDirectDraw::SetCooperativeLevel\n", r, ddResultToStr(r).c_str());

		enum1ctx ctx1 = {0};
		ctx1.count = 0;
		ctx1.pdd = pdd;
		ctx1.pd3d2 = nullptr;
		r = pd3d->EnumDevices(enumDevicesFormats1, &ctx1);
		if (r != DD_OK)
			printf("%d (%s) D3D::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), ctx1.count);
	}

#if(DIRECT3D_VERSION >= 0x0500)
	if (pd3d2)
	{
		printf("DX5:\n");
		enum1ctx ctx2 = {0};
		ctx2.count = 0;
		ctx2.pdd = pdd;
		ctx2.pd3d2 = pd3d2;
		r = pd3d2->EnumDevices(enumDevicesFormats1, &ctx2);
		if (r != DD_OK)
			printf("%d (%s) D3D2::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), ctx2.count);
	}
#endif

#if(DIRECT3D_VERSION >= 0x0600)
	if (pd3d3)
	{
		printf("DX6:\n");
		enum3ctx ctx = {0};
		ctx.pdd4 = pdd4;
		ctx.pd3d3 = pd3d3;
		r = pd3d3->EnumDevices(enumDevicesFormats3, &ctx);
		if (r != DD_OK)
			printf("%d (%s) D3D3::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), ctx.count);
	}
#endif

#if(DIRECT3D_VERSION >= 0x0700)
	if (pd3d7)
	{
		printf("DX7:\n");

		r = pdd7->SetCooperativeLevel(nullptr, DDSCL_NORMAL);
		if (r != DD_OK)
			printf("%d (%s) IDirectDraw7::SetCooperativeLevel\n", r, ddResultToStr(r).c_str());

		enum7ctx ctx = {0};
		ctx.pdd7 = pdd7;
		ctx.pd3d7 = pd3d7;
		r = pd3d7->EnumDevices(enumDevicesFormats7, &ctx);
		if (r != DD_OK)
			printf("%d (%s) D3D7::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), ctx.count);
	}
#endif
}

void printDDCapsAll(IDirectDraw *pdd, IDirectDraw2 *pdd2, IDirectDraw4 *pdd4, IDirectDraw7 *pdd7)
{
	HRESULT r;

	printf("DirectDraw caps:\n");

	printf("DX1:\n");
	DDCAPS_DX1 driverCaps1 = {0};
	driverCaps1.dwSize = sizeof(driverCaps1);
	DDCAPS_DX1 HELCaps1 = {0};
	HELCaps1.dwSize = sizeof(HELCaps1);
	r = pdd->GetCaps((DDCAPS*)&driverCaps1, (DDCAPS*)&HELCaps1);
	if (r != DD_OK)
		printf("%d (%s) IDirectDraw::GetCaps(DX1)\n", r, ddResultToStr(r).c_str());
	else
	{
		printf("Driver DDCAPS size %d\n", driverCaps1.dwSize);
		printDD1Caps(driverCaps1);

		printf("HEL DDCAPS size %d\n", HELCaps1.dwSize);
		printDD1Caps(HELCaps1);
	}

	DWORD numCC = 0;
	r = pdd->GetFourCCCodes(&numCC, nullptr);
	if (r != DD_OK)
		printf("%d (%s) IDirectDraw::GetFourCCCodes %d\n", r, ddResultToStr(r).c_str(), numCC);
	else
		printf("GetFourCCCodes %d\n", numCC);
	std::vector<DWORD> ccs(numCC);
	r = pdd->GetFourCCCodes(&numCC, &ccs[0]);
	if (r != DD_OK)
		printf("%d (%s) DD::GetFourCCCodes %d\n", r, ddResultToStr(r).c_str(), numCC);
	for (size_t i = 0; i < ccs.size(); i++)
		printf(" %2d: %X (%.4s)\n", (int)i, ccs[i], (const char *)&ccs[i]);

#if(DIRECTDRAW_VERSION >= 0x0200)
	DDCAPS_DX3 driverCaps3 = {0};
	driverCaps3.dwSize = sizeof(driverCaps3);
	DDCAPS_DX3 HELCaps3 = {0};
	HELCaps3.dwSize = sizeof(HELCaps3);
	if (pdd2)
	{
		printf("DX3:\n");
		r = pdd2->GetCaps((DDCAPS*)&driverCaps3, (DDCAPS*)&HELCaps3);
		if (r != DD_OK)
			printf("%d (%s) IDirectDraw2::GetCaps(DX3)\n", r, ddResultToStr(r).c_str());
		else
		{
			printf("Driver DDCAPS size %d\n", driverCaps3.dwSize);
			if (memcmp(&driverCaps3.dwCaps, &driverCaps1.dwCaps, driverCaps1.dwSize - sizeof(DWORD)))
				printDD1Caps(*(DDCAPS_DX1*)&driverCaps3);
			else
				printf(" starts same as DX1\n");
			printDD3Caps(driverCaps3);

			printf("HEL DDCAPS size %d\n", HELCaps3.dwSize);
			if (memcmp(&HELCaps3.dwCaps, &HELCaps1.dwCaps, HELCaps1.dwSize - sizeof(DWORD)))
				printDD1Caps(*(DDCAPS_DX1*)&HELCaps3);
			else
				printf(" starts same as DX1\n");
			printDD3Caps(HELCaps3);
		}

		DDSCAPS ddsCaps = { 0 };
		DWORD totalMem = 0;
		DWORD freeMem = 0;
		r = pdd2->GetAvailableVidMem(&ddsCaps, &totalMem, &freeMem);
		if (r != DD_OK)
			printf("%d (%s) DD2::GetAvailableVidMem total %u free %u\n", r, ddResultToStr(r).c_str(), totalMem, freeMem);
		else
			printf("DD2::GetAvailableVidMem total %u free %u\n", totalMem, freeMem);
	}
#endif

#if (DIRECTDRAW_VERSION >= 0x0500)
	DDCAPS_DX5 driverCaps5 = {0};
	driverCaps5.dwSize = sizeof(driverCaps5);
	DDCAPS_DX5 HELCaps5 = {0};
	HELCaps5.dwSize = sizeof(HELCaps5);
	if (pdd2)
	{
		printf("DX5:\n");
		r = pdd2->GetCaps((DDCAPS*)&driverCaps5, (DDCAPS*)&HELCaps5);
		if (r != DD_OK)
			printf("%d (%s) IDirectDraw2::GetCaps(DX5)\n", r, ddResultToStr(r).c_str());
		else
		{
			printf("Driver DDCAPS size %d\n", driverCaps5.dwSize);
			if (memcmp(&driverCaps5.dwCaps, &driverCaps3.dwCaps, driverCaps3.dwSize - sizeof(DWORD)))
			{
				printDD1Caps(*(DDCAPS_DX1*)&driverCaps5);
				printDD3Caps(*(DDCAPS_DX3*)&driverCaps5);
			}
			else
				printf(" starts same as DX3\n");
			printDD5Caps(driverCaps5);

			printf("HEL DDCAPS size %d\n", HELCaps5.dwSize);
			if (memcmp(&HELCaps5.dwCaps, &HELCaps3.dwCaps, HELCaps3.dwSize - sizeof(DWORD)))
			{
				printDD1Caps(*(DDCAPS_DX1*)&HELCaps5);
				printDD3Caps(*(DDCAPS_DX3*)&HELCaps5);
			}
			else
				printf(" starts same as DX3\n");
			printDD5Caps(HELCaps5);
		}
	}
#endif

#if (DIRECTDRAW_VERSION >= 0x0600)
	DDCAPS_DX6 driverCaps6 = {0};
	driverCaps6.dwSize = sizeof(driverCaps6);
	DDCAPS_DX6 HELCaps6 = {0};
	HELCaps6.dwSize = sizeof(HELCaps6);
	if (pdd4)
	{
		printf("DX6:\n");
		r = pdd4->GetCaps((DDCAPS*)&driverCaps6, (DDCAPS*)&HELCaps6);
		if (r != DD_OK)
			printf("%d (%s) IDirectDraw4::GetCaps(DX6)\n", r, ddResultToStr(r).c_str());
		else
		{
			printf("Driver DDCAPS size %d\n", driverCaps6.dwSize);
			if (memcmp(&driverCaps6.dwCaps, &driverCaps5.dwCaps, driverCaps5.dwSize - sizeof(DWORD)))
			{
				printDD1Caps(*(DDCAPS_DX1*)&driverCaps6);
				printDD3Caps(*(DDCAPS_DX3*)&driverCaps6);
				printDD5Caps(*(DDCAPS_DX5*)&driverCaps6);
			}
			else
				printf(" starts same as DX5\n");
			printDD6Caps(driverCaps6);

			printf("HEL DDCAPS size %d\n", HELCaps6.dwSize);
			if (memcmp(&HELCaps6.dwCaps, &HELCaps5.dwCaps, HELCaps5.dwSize - sizeof(DWORD)))
			{
				printDD1Caps(*(DDCAPS_DX1*)&HELCaps6);
				printDD3Caps(*(DDCAPS_DX3*)&HELCaps6);
				printDD5Caps(*(DDCAPS_DX5*)&HELCaps6);
			}
			else
				printf(" starts same as DX5\n");
			printDD6Caps(HELCaps6);
		}

#if 0
		numCC = 0;
		r = pdd4->GetFourCCCodes(&numCC, nullptr);
		printf("%d (%s) DD4::GetFourCCCodes %d\n", r, ddResultToStr(r).c_str(), numCC);
		ccs.resize(numCC);
		r = pdd4->GetFourCCCodes(&numCC, ccs.data());
		if (r) printf("%d (%s) DD4::GetFourCCCodes %d\n", r, ddResultToStr(r).c_str(), numCC);
		for (size_t i = 0; i < ccs.size(); i++)
			printf(" %2d: %X (%.4s)\n", (int)i, ccs[i], &ccs[i]);
#endif

	}
#endif

#if (DIRECTDRAW_VERSION >= 0x0700)
	DDCAPS_DX7 driverCaps7 = {0};
	driverCaps7.dwSize = sizeof(driverCaps7);
	DDCAPS_DX7 HELCaps7 = {0};
	HELCaps7.dwSize = sizeof(HELCaps7);
	if (pdd7)
	{
		printf("DX7:\n");
		r = pdd7->GetCaps((DDCAPS*)&driverCaps7, (DDCAPS*)&HELCaps7);
		if (r != DD_OK)
			printf("%d (%s) IDirectDraw7::GetCaps(DX7)\n", r, ddResultToStr(r).c_str());
		else
		{
			printf("Driver DDCAPS size %d\n", driverCaps7.dwSize);
			if (memcmp(&driverCaps7.dwCaps, &driverCaps6.dwCaps, driverCaps6.dwSize - sizeof(DWORD)))
			{
				printDD1Caps(*(DDCAPS_DX1*)&driverCaps7);
				printDD3Caps(*(DDCAPS_DX3*)&driverCaps7);
				printDD5Caps(*(DDCAPS_DX5*)&driverCaps7);
				printDD6Caps(*(DDCAPS_DX6*)&driverCaps7);
			}
			else
				printf(" same as DX6\n");

			printf("HEL DDCAPS size %d\n", HELCaps7.dwSize);
			if (memcmp(&HELCaps7.dwCaps, &HELCaps6.dwCaps, HELCaps6.dwSize - sizeof(DWORD)))
			{
				printDD1Caps(*(DDCAPS_DX1*)&HELCaps7);
				printDD3Caps(*(DDCAPS_DX3*)&HELCaps7);
				printDD5Caps(*(DDCAPS_DX5*)&HELCaps7);
				printDD6Caps(*(DDCAPS_DX6*)&HELCaps7);
			}
			else
				printf(" same as DX6\n");
		}

		DWORD numCC7 = 0;
		r = pdd7->GetFourCCCodes(&numCC7, nullptr);
		if (r != DD_OK)
			printf("%d (%s) IDirectDraw7::GetFourCCCodes %d\n", r, ddResultToStr(r).c_str(), numCC7);
		std::vector<DWORD> ccs7(numCC7);
		r = pdd7->GetFourCCCodes(&numCC7, &ccs7[0]);
		if (r != DD_OK)
			printf("%d (%s) DD7::GetFourCCCodes %d\n", r, ddResultToStr(r).c_str(), numCC7);
		if (numCC7 != numCC && ccs != ccs7)
		{
			printf("DD7::GetFourCCCodes %d\n", numCC7);
			for (size_t i = 0; i < ccs7.size(); i++)
				printf(" %2d: %X (%.4s)\n", (int)i, ccs7[i], (const char *)&ccs7[i]);
		}
	}
#endif
}

HRESULT CALLBACK enumDevicesCaps (GUID FAR* lpGUID, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC descHW, LPD3DDEVICEDESC descHEL, LPVOID user)
{
	uint64_t guid[2] = { 0 };
	if (lpGUID)
		memcpy(guid, lpGUID, sizeof(GUID));

	printf(" guid %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDeviceName, lpDeviceDescription);
	printD3DDCaps("HW", *descHW);
	printD3DDCaps("HEL", *descHEL);
	(*(int*)user)++;
	return DDENUMRET_OK;
}

HRESULT CALLBACK enumDevicesCaps7 (LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 desc, LPVOID user)
{
	uint64_t guid[2] = { 0 };
	if (desc)
		memcpy(guid, &desc->deviceGUID, sizeof(GUID));

	printf("guid %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDeviceName, lpDeviceDescription);
	printD3DDCaps7(*desc);
	(*(int*)user)++;
	return DDENUMRET_OK;
}

void printD3DDevicesCaps(IDirect3D* pd3d, IDirect3D2* pd3d2, IDirect3D3* pd3d3, IDirect3D7* pd3d7)
{
	HRESULT r;
	int count = 0;

	printf("D3D Device caps:\n");

#if(DIRECT3D_VERSION >= 0x0200)
	if (pd3d)
	{
		printf("DX2:\n");
		count = 0;
		r = pd3d->EnumDevices(enumDevicesCaps, &count);
		if (r != DD_OK)
			printf("%d (%s) IDirect3D::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);
	}
#endif
#if(DIRECT3D_VERSION >= 0x0500)
	if (pd3d2)
	{
		printf("\nDX5:\n");
		count = 0;
		r = pd3d2->EnumDevices(enumDevicesCaps, &count);
		if (r != DD_OK)
			printf("%d (%s) IDirect3D2::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);
	}
#endif
#if(DIRECT3D_VERSION >= 0x0600)
	if (pd3d3)
	{
		printf("\nDX6:\n");
		count = 0;
		r = pd3d3->EnumDevices(enumDevicesCaps, &count);
		if (r != DD_OK)
			printf("%d (%s) D3D3::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);
	}
#endif
#if(DIRECT3D_VERSION >= 0x0700)
	if (pd3d7)
	{
		printf("\nDX7:\n");
		count = 0;
		r = pd3d7->EnumDevices(enumDevicesCaps7, &count);
		if (r != DD_OK)
			printf("%d (%s) D3D7::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);
	}
#endif
}

BOOL CALLBACK ddEnumPrint(GUID FAR* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext)
{
	uint64_t guid[2] = { 0 };
	if (lpGUID)
		memcpy(guid, lpGUID, sizeof(GUID));

	printf(" guid %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDriverName, lpDriverDescription);
	return TRUE;
}

BOOL CALLBACK ddEnumExPrint(GUID FAR* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm)
{
	uint64_t guid[2] = { 0 };
	if (lpGUID)
		memcpy(guid, lpGUID, sizeof(GUID));

	printf(" guid: %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\" monitor %p\n", guid[0], guid[1], lpDriverName, lpDriverDescription, hm);
	return TRUE;
}

int main()
{
	printf("dxinfo compiled for dd 0x%X d3d 0x%X\n", DIRECTDRAW_VERSION, DIRECT3D_VERSION);
	HRESULT r;

	HMODULE lib = LoadLibraryA("ddraw.dll");
	printf("Load ddraw.dll\n");
	if (!lib)
	{
		printf("GetLastError %u\n", GetLastError());
		return -1;
	}

	//DX1
	typedef HRESULT (WINAPI *PFN_DirectDrawEnumerateA)( LPDDENUMCALLBACKA lpCallback, LPVOID lpContext );
	typedef HRESULT (WINAPI *PFN_DirectDrawEnumerateW)( LPDDENUMCALLBACKW lpCallback, LPVOID lpContext );
	typedef HRESULT (WINAPI *PFN_DirectDrawCreate)( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );
	//DX5
	typedef HRESULT (WINAPI *PFN_DirectDrawEnumerateExA)( LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
	typedef HRESULT (WINAPI *PFN_DirectDrawEnumerateExW)( LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags);
	//DX7
	typedef HRESULT (WINAPI *PFN_DirectDrawCreateEx)( GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter );

#define X(name) PFN_##name p##name = (PFN_##name)GetProcAddress(lib, #name);\
	printf(#name " \t%s\n", p##name ? "supported" : "missing");

	X(DirectDrawEnumerateA)
	X(DirectDrawEnumerateW)
	X(DirectDrawCreate)
	X(DirectDrawEnumerateExA)
	X(DirectDrawEnumerateExW)
	X(DirectDrawCreateEx)
#undef X

	if (!pDirectDrawEnumerateA || !pDirectDrawCreate)
	{
		printf("Basic functions not supported\n");
		return -1;
	}

	printf("\nDirectDraw devices:\n");
	printf("DX1:\n");

	r = pDirectDrawEnumerateA(ddEnumPrint, nullptr);
	if (r != DD_OK)
		printf("%d (%s) DirectDrawEnumerateA\n", r, ddResultToStr(r).c_str());

#if(DIRECTDRAW_VERSION >= 0x0500)
	if(pDirectDrawEnumerateExA)
	{
		printf("DX5: (ATTACHEDSECONDARY|DETACHEDSECONDARY|NONDISPLAY)\n");
		r = pDirectDrawEnumerateExA(ddEnumExPrint, nullptr, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
		if (r != DD_OK)
			printf("%d (%s) DirectDrawEnumerateExA\n", r, ddResultToStr(r).c_str());
	}
#endif

	IDirectDraw* pdd = nullptr;
	IDirectDraw2* pdd2 = nullptr;
	IDirectDraw4* pdd4 = nullptr;
	IDirectDraw7* pdd7 = nullptr;

	r = pDirectDrawCreate(nullptr, &pdd, nullptr);
	if (!pdd || r != DD_OK)
	{
		printf("%d (%s) DirectDrawCreate %p\n", r, ddResultToStr(r).c_str(), pdd);
		return -1;
	}
	printf("\nCreated primary DD device\n");

#if(DIRECTDRAW_VERSION >= 0x0200)
	r = pdd->QueryInterface(IID_IDirectDraw2, (void**)&pdd2);
	if (!pdd2 || r != DD_OK)
		printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirectDraw2) %p\n", r, ddResultToStr(r).c_str(), pdd2);
	else
		printf("QueryInterface(DD2) supported\n");
#endif

#if(DIRECTDRAW_VERSION >= 0x0600)
	r = pdd->QueryInterface(IID_IDirectDraw4, (void**)&pdd4);
	if (!pdd4 || r != DD_OK)
		printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirectDraw4) %p\n", r, ddResultToStr(r).c_str(), pdd4);
	else
		printf("QueryInterface(DD6) supported\n");
#endif

#if(DIRECTDRAW_VERSION >= 0x0700)
	if (pDirectDrawCreateEx)
	{
		r = pDirectDrawCreateEx(nullptr, (void**)&pdd7, IID_IDirectDraw7, nullptr);
		if (!pdd7 || r != DD_OK)
			printf("%d (%s) DirectDrawCreateEx(DD7) %p\n", r, ddResultToStr(r).c_str(), pdd7);
		else
			printf("DirectDrawCreateEx(DD7) supported\n");
	}
#endif

	printDDCapsAll(pdd, pdd2, pdd4, pdd7);
	//printModes(pdd, pdd2, pdd4, pdd7);
	//printSurfaces(pdd, pdd2, pdd4, pdd7);

	printf("\n");

	IDirect3D* pd3d = nullptr;
	IDirect3D2* pd3d2 = nullptr;
	IDirect3D3* pd3d3 = nullptr;
	IDirect3D7* pd3d7 = nullptr;

#if(DIRECT3D_VERSION >= 0x0200)
	r = pdd->QueryInterface(IID_IDirect3D, (void**)&pd3d);
	if (!pd3d || r != DD_OK)
		printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirect3D) %p\n", r, ddResultToStr(r).c_str(), pd3d);
	else
		printf("QueryInterface(IDirect3D) supported\n");
#endif
#if(DIRECT3D_VERSION >= 0x0500)
	r = pdd->QueryInterface(IID_IDirect3D2, (void**)&pd3d2);
	if (!pd3d2 || r != DD_OK)
		printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirect3D2) %p\n", r, ddResultToStr(r).c_str(), pd3d2);
	else
		printf("QueryInterface(D3D5) supported\n");
#endif
#if(DIRECT3D_VERSION >= 0x0600)
	r = pdd->QueryInterface(IID_IDirect3D3, (void**)&pd3d3);
	if (!pd3d3 || r != DD_OK)
		printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirect3D3) %p\n", r, ddResultToStr(r).c_str(), pd3d3);
	else
		printf("QueryInterface(D3D6) supported\n");
#endif
#if(DIRECT3D_VERSION >= 0x0700)
	r = pdd7->QueryInterface(IID_IDirect3D7, (void**)&pd3d7);
	if (!pd3d7 || r != DD_OK)
		printf("%d (%s) IDirectDraw7::QueryInterface(IID_IDirect3D7) %p\n", r, ddResultToStr(r).c_str(), pd3d7);
	else
		printf("DD7::QueryInterface(D3D7) supported\n");
#endif

	printD3DDevices(pd3d, pd3d2, pd3d3, pd3d7);
	printD3DFormats(pdd, pdd4, pdd7, pd3d, pd3d2, pd3d3, pd3d7);
	printD3DDevicesCaps(pd3d, pd3d2, pd3d3, pd3d7);

	if (pd3d7)
		VerboseRelease(pd3d7, "IDirect3D7");
	
	if (pdd7)
		VerboseRelease(pdd7, "IDirectDraw7");

	if (pd3d)
		VerboseRelease(pd3d, "IDirect3D");

	if (pd3d2)
		VerboseRelease(pd3d2, "IDirect3D2");

	if (pd3d3)
		VerboseRelease(pd3d3, "IDirect3D3");

	if (pdd2)
		VerboseRelease(pdd2, "IDirectDraw2");

	if (pdd4)
		VerboseRelease(pdd2, "IDirectDraw4");

	VerboseRelease(pdd, "IDirectDraw");

	return 0;
}
