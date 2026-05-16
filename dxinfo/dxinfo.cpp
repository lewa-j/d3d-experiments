
#include <stdio.h>
#include <inttypes.h>
#include <string>
#include <vector>

#define INITGUID
#define DIRECTDRAW_VERSION 0x0700
#define DIRECT3D_VERSION 0x0700

#if(DIRECT3D_VERSION < 0x0500)
#define D3DCOLOR_MONO   1
#define D3DCOLOR_RGB    2
typedef unsigned long D3DCOLORMODEL;
#endif

#include <d3d.h>

#pragma comment(lib, "ddraw.lib")

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
	char buff[256]{};
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
	printf("%d %s::Release\n", c, name);
	return c;
}

#ifndef DDCAPS_STEREOVIEW
#define DDCAPS_STEREOVIEW 0x00040000l
#endif
#ifndef DDCAPS2_PUREHAL
#define DDCAPS2_PUREHAL 0x08000000L
#endif
#ifndef DDSVCAPS_ENIGMA
#define DDSVCAPS_ENIGMA 0x00000001l
#define DDSVCAPS_FLICKER 0x00000002l
#define DDSVCAPS_REDBLUE 0x00000004l
#define DDSVCAPS_SPLIT 0x00000008l
#endif

#undef DDSCAPS_PRIMARYSURFACELEFT
#define DDSCAPS_PRIMARYSURFACELEFT 0x00000400l

void printDDSCaps(const char* name, DWORD c)
{
	printf("%s %X(", name, c);
#define X(name) if (c & DDSCAPS_##name) printf(#name " ");
X(RESERVED1)
X(ALPHA)
X(BACKBUFFER)
X(COMPLEX)
X(FLIP)
X(FRONTBUFFER)
X(OFFSCREENPLAIN)
X(OVERLAY)
X(PALETTE)
X(PRIMARYSURFACE)
//DDSCAPS_RESERVED3
X(PRIMARYSURFACELEFT)
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
X(VIDEOPORT)
X(LOCALVIDMEM)
X(NONLOCALVIDMEM)
X(STANDARDVGAMODE)
X(OPTIMIZED)

#undef X
	printf(")");
}

void printPixelFormat(const char* name, DDPIXELFORMAT& pf)
{
	printf("%s size %d flags %X", name, pf.dwSize, pf.dwFlags);

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

	printf("\n");
}

void printDDCaps(const char* name, DDCAPS_DX1& c)
{
	printf("%s DDCAPS size %d\n", name, c.dwSize);
	printf(" Caps %X(", c.dwCaps);
#define X(name) if (c.dwCaps & DDCAPS_##name) printf(#name " ");
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

	printf(" Caps2 %X(", c.dwCaps2);
#define X(name) if (c.dwCaps2 & DDCAPS2_##name) printf(#name " ");
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
	printf(")\n");

	printf(" CKeyCaps %X(", c.dwCKeyCaps);
#define X(name) if (c.dwCKeyCaps & DDCKEYCAPS_##name) printf(#name " ");
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

	printf(" FXCaps %X(", c.dwFXCaps);
#define X(name) if (c.dwFXCaps & DDFXCAPS_##name) printf(#name " ");
	X(BLTALPHA)
	//2?
	X(OVERLAYALPHA)
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
	X(OVERLAYDEINTERLACE)
//#define DDFXCAPS_BLTFILTER              DDFXCAPS_BLTARITHSTRETCHY
//#define DDFXCAPS_OVERLAYFILTER          DDFXCAPS_OVERLAYARITHSTRETCHY
#undef X
	printf(")\n");

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
	printf(")\n");

	printf(" PalCaps %X(", c.dwPalCaps);
#define X(name) if (c.dwPalCaps & DDPCAPS_##name) printf(#name " ");
	X(4BIT)
	X(8BITENTRIES)
	X(8BIT)
//#define DDPCAPS_INITIALIZE 0x00000008l ?
	X(PRIMARYSURFACE)
	X(PRIMARYSURFACELEFT)
	X(ALLOW256)
	X(VSYNC)
	X(1BIT)
	X(2BIT)
	X(ALPHA)
#undef X
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
	
	printf(" AlphaBltConstBitDepths %X AlphaBltPixelBitDepths %X AlphaBltSurfaceBitDepths %X AlphaOverlayConstBitDepths %X AlphaOverlayPixelBitDepths %X AlphaOverlaySurfaceBitDepths %X ZBufferBitDepths %X\n",
		c.dwAlphaBltConstBitDepths, c.dwAlphaBltPixelBitDepths, c.dwAlphaBltSurfaceBitDepths, c.dwAlphaOverlayConstBitDepths, c.dwAlphaOverlayPixelBitDepths, c.dwAlphaOverlaySurfaceBitDepths, c.dwZBufferBitDepths);
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
#undef X
#if(DIRECT3D_VERSION >= 0x0600)
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
	X(MINFPOINT)
	X(MINFLINEAR)
	X(MINFANISOTROPIC)
	X(MIPFPOINT)
	X(MIPFLINEAR)
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

void printD3DDCaps(const char* name, const D3DDEVICEDESC &c)
{
	printf("%s caps: flags %X\n", name, c.dwFlags);
	if (c.dwFlags & D3DDD_COLORMODEL)
		printf(" ColorModel %X\n", c.dcmColorModel);
	if (c.dwFlags * D3DDD_DEVCAPS)
	{
		printf(" DevCaps %X (", c.dwDevCaps);
#define X(name) if (c.dwDevCaps & D3DDEVCAPS_##name) printf(#name " ");
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
#undef X
#if(DIRECT3D_VERSION >= 0x0600)
#define DEVCAPS_VALID_MASK 0xFFFF
#elif(DIRECT3D_VERSION >= 0x0500)
#define DEVCAPS_VALID_MASK 0x1FFF
#else
#define DEVCAPS_VALID_MASK 0x3FF
#endif
		if (c.dwDevCaps & ~DEVCAPS_VALID_MASK)
			printf("Unknown flags %X", c.dwDevCaps & ~DEVCAPS_VALID_MASK);
		printf(")\n");
	}
	if (c.dwFlags & D3DDD_TRANSFORMCAPS)
		printf(" TransformCaps size %d caps %X\n", c.dtcTransformCaps.dwSize, c.dtcTransformCaps.dwCaps);
	if (c.dwFlags & D3DDD_LIGHTINGCAPS)
		printf(" LightingCaps size %d caps %X model %X lights %d\n", c.dlcLightingCaps.dwSize, c.dlcLightingCaps.dwCaps, c.dlcLightingCaps.dwNumLights, c.dlcLightingCaps.dwNumLights);
	if (c.dwFlags & D3DDD_BCLIPPING)
		printf(" bClipping %d\n", c.bClipping);
	if (c.dwFlags & D3DDD_LINECAPS)
		printD3DPrimCaps("LineCaps", c.dpcLineCaps);
	if (c.dwFlags & D3DDD_TRICAPS)
		printD3DPrimCaps("TriCaps", c.dpcTriCaps);
	if (c.dwFlags & D3DDD_DEVICERENDERBITDEPTH)
		printf(" DeviceRenderBitDepth %X\n", c.dwDeviceRenderBitDepth);
	if (c.dwFlags & D3DDD_DEVICEZBUFFERBITDEPTH)
		printf(" DeviceZBufferBitDepth %X\n", c.dwDeviceZBufferBitDepth);
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

void printModesDevices(IDirectDraw *pdd, IDirectDraw7 *pdd7)
{
	HRESULT r;

	IDirectDraw2* pdd2 = nullptr;
	r = pdd->QueryInterface(IID_IDirectDraw2, (void**)&pdd2);
	printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirectDraw2) %p\n", r, ddResultToStr(r).c_str(), pdd2);
	IDirectDraw4* pdd4 = nullptr;
	r = pdd->QueryInterface(IID_IDirectDraw4, (void**)&pdd4);
	printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirectDraw4) %p\n", r, ddResultToStr(r).c_str(), pdd4);

	DDCAPS_DX1 driverCaps{};
	driverCaps.dwSize = sizeof(driverCaps);
	DDCAPS_DX1 HELCaps{};
	HELCaps.dwSize = sizeof(HELCaps);
	r = pdd->GetCaps((DDCAPS*)&driverCaps, (DDCAPS*)&HELCaps);
	printf("%d (%s) IDirectDraw::GetCaps\n", r, ddResultToStr(r).c_str());
	if (r == DD_OK)
	{
		printDDCaps("Driver", driverCaps);
		printDDCaps("HEL", HELCaps);
	}
	//TODO DD4 caps
	//DOTO DD7 caps

	DWORD numCC = 0;
	r = pdd->GetFourCCCodes(&numCC, nullptr);
	printf("%d (%s) IDirectDraw::GetFourCCCodes %d\n", r, ddResultToStr(r).c_str(), numCC);
	std::vector<DWORD> ccs(numCC);
	r = pdd->GetFourCCCodes(&numCC, ccs.data());
	if (r) printf("%d (%s) DD::GetFourCCCodes %d\n", r, ddResultToStr(r).c_str(), numCC);
	for (size_t i = 0; i < ccs.size(); i++)
		printf(" %2zu: %X (%.4s)\n", i, ccs[i], (const char *)&ccs[i]);
#if 0
	numCC = 0;
	r = pdd4->GetFourCCCodes(&numCC, nullptr);
	printf("%d (%s) DD4::GetFourCCCodes %d\n", r, ddResultToStr(r).c_str(), numCC);
	ccs.resize(numCC);
	r = pdd4->GetFourCCCodes(&numCC, ccs.data());
	if (r) printf("%d (%s) DD4::GetFourCCCodes %d\n", r, ddResultToStr(r).c_str(), numCC);
	for (size_t i = 0; i < ccs.size(); i++)
		printf(" %2zu: %X (%.4s)\n", i, ccs[i], &ccs[i]);
#endif

	DDSCAPS ddsCaps{ 0 };
	DWORD totalMem = 0;
	DWORD freeMem = 0;
	r = pdd2->GetAvailableVidMem(&ddsCaps, &totalMem, &freeMem);
	printf("%d (%s) DD2::GetAvailableVidMem total %u free %u\n", r, ddResultToStr(r).c_str(), totalMem, freeMem);

	auto enumModes = [](LPDDSURFACEDESC lpDDSDesc, LPVOID lpContext) -> HRESULT
		{
			printf(" %2d: %dx%d %dHz %dbit\n", *(int*)lpContext, lpDDSDesc->dwWidth, lpDDSDesc->dwHeight, lpDDSDesc->dwRefreshRate, lpDDSDesc->ddpfPixelFormat.dwRGBBitCount);
			printf("  flags %X pitch %d ", lpDDSDesc->dwFlags, lpDDSDesc->lPitch);
			printPixelFormat("pf", lpDDSDesc->ddpfPixelFormat);
			(*(int*)lpContext)++;
			return DDENUMRET_OK;
		};

	int count = 0;
	r = pdd->EnumDisplayModes(DDEDM_REFRESHRATES, nullptr, &count, enumModes);
	printf("%d (%s) IDirectDraw::EnumDisplayModes count %d\n", r, ddResultToStr(r).c_str(), count);
#if 0
	if (pdd2)
	{
		count = 0;
		r = pdd2->EnumDisplayModes(DDEDM_REFRESHRATES, nullptr, &count, enumModes);
		printf("%d (%s) DD2::EnumDisplayModes count %d\n", r, ddResultToStr(r).c_str(), count);
	}
#endif
	auto enumSurf = [](LPDIRECTDRAWSURFACE lpDDSurface, LPDDSURFACEDESC lpDesc, LPVOID lpContext) ->HRESULT
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
		};

	count = 0;
	r = pdd->EnumSurfaces(DDENUMSURFACES_DOESEXIST | DDENUMSURFACES_ALL, nullptr, &count, enumSurf);
	printf("%d (%s) IDirectDraw::EnumSurfaces(DDENUMSURFACES_DOESEXIST | DDENUMSURFACES_ALL) count %d\n", r, ddResultToStr(r).c_str(), count);

	count = 0;
	DDSURFACEDESC ddsd{ 0 };
	ddsd.dwSize = sizeof(ddsd);
	r = pdd->EnumSurfaces(DDENUMSURFACES_CANBECREATED | DDENUMSURFACES_MATCH, &ddsd, &count, enumSurf);
	printf("%d (%s) IDirectDraw::EnumSurfaces(DDENUMSURFACES_CANBECREATED | DDENUMSURFACES_MATCH) count %d\n", r, ddResultToStr(r).c_str(), count);
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

	auto enumDevices = [](GUID FAR* lpGUID, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC descHW, LPD3DDEVICEDESC descHEL, LPVOID lpContext) -> HRESULT
		{
			uint64_t guid[2]{ 0 };
			if (lpGUID)
				memcpy(guid, lpGUID, sizeof(GUID));

			printf("EnumDevices: %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDeviceName, lpDeviceDescription);

			(*(int*)lpContext)++;
			return DDENUMRET_OK;
		};

	printf("DX2:\n");
	IDirect3D* pd3d = nullptr;
	r = pdd->QueryInterface(IID_IDirect3D, (void**)&pd3d);
	printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirect3D) %p\n", r, ddResultToStr(r).c_str(), pd3d);
	if (pd3d)
	{
		count = 0;
		r = pd3d->EnumDevices(enumDevices, &count);
		printf("%d (%s) D3D::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);
	}
#if(DIRECT3D_VERSION >= 0x0500)
	printf("DX5:\n");
	IDirect3D2* pd3d2 = nullptr;
	r = pdd->QueryInterface(IID_IDirect3D2, (void**)&pd3d2);
	printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirect3D2) %p\n", r, ddResultToStr(r).c_str(), pd3d2);
	if (pd3d2)
	{
		count = 0;
		r = pd3d2->EnumDevices(enumDevices, &count);
		printf("%d (%s) D3D2::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);
	}
#endif
#if(DIRECT3D_VERSION >= 0x0600)
	printf("DX6:\n");
	IDirect3D3* pd3d3 = nullptr;
	r = pdd->QueryInterface(IID_IDirect3D3, (void**)&pd3d3);
	printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirect3D3) %p\n", r, ddResultToStr(r).c_str(), pd3d3);
	if (pd3d3)
	{
		count = 0;
		r = pd3d3->EnumDevices(enumDevices, &count);
		printf("%d (%s) D3D3::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);
	}
#endif
#if(DIRECT3D_VERSION >= 0x0700)
	auto enumDevices7 = [](LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 descHW, LPVOID lpContext) -> HRESULT
		{
			uint64_t guid[2]{ 0 };
			if (descHW)
				memcpy(guid, &descHW->deviceGUID, sizeof(GUID));

			printf("EnumDevices: %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDeviceName, lpDeviceDescription);

			(*(int*)lpContext)++;
			return DDENUMRET_OK;
		};

	if (pdd7)
	{
		printf("DX7:\n");
		IDirect3D7* pd3d7 = nullptr;
		r = pdd7->QueryInterface(IID_IDirect3D7, (void**)&pd3d7);
		printf("%d (%s) IDirectDraw7::QueryInterface(IID_IDirect3D7) %p\n", r, ddResultToStr(r).c_str(), pd3d7);
		if (pd3d7)
		{
			count = 0;
			r = pd3d7->EnumDevices(enumDevices7, &count);
			printf("%d (%s) D3D7::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);
		}
	}
#endif

	if (pd3d)
	{
		printf("DX2 formats:\n");

		struct enum1ctx {
			int count = 0;
			IDirectDraw* pdd = nullptr;
			IDirect3D* pd3d = nullptr;
		} ctx1{};
		auto enumDevices1 = [](GUID FAR* lpGUID, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC descHW, LPD3DDEVICEDESC descHEL, LPVOID lpContext) -> HRESULT
			{
				enum1ctx& ctx = *(enum1ctx*)lpContext;
				uint64_t guid[2]{ 0 };
				if (lpGUID)
					memcpy(guid, lpGUID, sizeof(GUID));

				printf("EnumDevices: %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDeviceName, lpDeviceDescription);

				DDSURFACEDESC ddsd{};
				ddsd.dwSize = sizeof(ddsd);
				ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
				ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE;
				ddsd.dwWidth = 256;
				ddsd.dwHeight = 256;
				IDirectDrawSurface* surf = nullptr;
				HRESULT r = ctx.pdd->CreateSurface(&ddsd, &surf, nullptr);
				printf("%d (%s) IDirectDraw::CreateSurface(3DDEVICE) %p\n", r, ddResultToStr(r).c_str(), surf);
				if (surf)
				{
					IDirect3DDevice* pd3dd = nullptr;
					r = surf->QueryInterface(*lpGUID, (void**)&pd3dd);
					printf("%d (%s) IDirectDrawSurface::QueryInterface(IDirect3DDevice) %p\n", r, ddResultToStr(r).c_str(), pd3dd);
					if (pd3dd)
					{
						auto enumDDSD = [](LPDDSURFACEDESC lpDdsd, LPVOID lpContext) -> HRESULT
							{
								printf(" size %d flags %X", lpDdsd->dwSize, lpDdsd->dwFlags);
								if (lpDdsd->dwFlags & DDSD_CAPS)
									printDDSCaps(" caps", lpDdsd->ddsCaps.dwCaps);
								if (lpDdsd->dwFlags & DDSD_PIXELFORMAT)
									printPixelFormat(" pf", lpDdsd->ddpfPixelFormat);
								(*(int*)lpContext)++;
								return DDENUMRET_OK;
							};

						int fmtCount = 0;
						r = pd3dd->EnumTextureFormats(enumDDSD, &fmtCount);
						printf("%d (%s) D3DD::EnumTextureFormats count %d\n", r, ddResultToStr(r).c_str(), fmtCount);

						VerboseRelease(pd3dd, "IDirect3DDevice");
					}

					VerboseRelease(surf, "IDirectDrawSurface");
				}

				ctx.count++;
				return DDENUMRET_OK;
			};

		r = pdd->SetCooperativeLevel(nullptr, DDSCL_NORMAL);
		printf("%d (%s) IDirectDraw::SetCooperativeLevel\n", r, ddResultToStr(r).c_str());

		ctx1.count = 0;
		ctx1.pdd = pdd;
		ctx1.pd3d = pd3d;
		r = pd3d->EnumDevices(enumDevices1, &ctx1);
		printf("%d (%s) D3D::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), ctx1.count);
	}

#if(DIRECT3D_VERSION >= 0x0600)
	if (pd3d3)
	{
		printf("DX6 formats:\n");

		struct enum3ctx {
			int count = 0;
			IDirectDraw4* pdd4 = nullptr;
			IDirect3D3* pd3d3 = nullptr;
		} ctx{};
		auto enumDevices3 = [](GUID FAR* lpGUID, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC descHW, LPD3DDEVICEDESC descHEL, LPVOID lpContext) -> HRESULT
			{
				enum3ctx& ctx = *(enum3ctx*)lpContext;
				uint64_t guid[2]{ 0 };
				if (lpGUID)
					memcpy(guid, lpGUID, sizeof(GUID));

				printf("EnumDevices: %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDeviceName, lpDeviceDescription);

				DDSURFACEDESC2 ddsd{};
				ddsd.dwSize = sizeof(ddsd);
				ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
				ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE;
				ddsd.dwWidth = 256;
				ddsd.dwHeight = 256;
				IDirectDrawSurface4* surf = nullptr;
				HRESULT r = ctx.pdd4->CreateSurface(&ddsd, &surf, nullptr);
				printf("%d (%s) IDirectDraw4::CreateSurface(3DDEVICE) %p\n", r, ddResultToStr(r).c_str(), surf);

				auto enumFmt = [](LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext) -> HRESULT
					{
						printPixelFormat(" pf", *lpDDPixFmt);
						(*(int*)lpContext)++;
						return DDENUMRET_OK;
					};

				if (surf)
				{
					IDirect3DDevice3* pd3dd = nullptr;
					r = ctx.pd3d3->CreateDevice(*lpGUID, surf, &pd3dd, nullptr);
					printf("%d (%s) D3D3::CreateDevice %p\n", r, ddResultToStr(r).c_str(), pd3dd);
					if (pd3dd)
					{
						int fmtCount = 0;
						r = pd3dd->EnumTextureFormats(enumFmt, &fmtCount);
						printf("%d (%s) D3DD3::EnumTextureFormats count %d\n", r, ddResultToStr(r).c_str(), fmtCount);

						VerboseRelease(pd3dd, "IDirect3DDevice");
					}

					VerboseRelease(surf, "IDirectDrawSurface");
				}

				int fmtCount = 0;
				r = ctx.pd3d3->EnumZBufferFormats(*lpGUID, enumFmt, &fmtCount);
				printf("%d (%s) D3D3::EnumZBufferFormats count %d\n", r, ddResultToStr(r).c_str(), fmtCount);

				ctx.count++;
				return DDENUMRET_OK;
			};
		ctx.pdd4 = pdd4;
		ctx.pd3d3 = pd3d3;
		r = pd3d3->EnumDevices(enumDevices3, &ctx);
		printf("%d (%s) D3D3::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), ctx.count);
	}
#endif

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
}

int main()
{
	HRESULT r;

	r = DirectDrawEnumerateA([](GUID FAR* lpGUID, LPSTR  lpDriverDescription, LPSTR  lpDriverName, LPVOID lpContext) -> BOOL
		{
			uint64_t guid[2]{ 0 };
			if (lpGUID)
				memcpy(guid, lpGUID, sizeof(GUID));

			printf("DirectDrawEnumerateA: %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDriverName, lpDriverDescription);
			return TRUE;
		}, nullptr);
	printf("%d (%s) DirectDrawEnumerateA\n", r, ddResultToStr(r).c_str());

	r = DirectDrawEnumerateExA([](GUID FAR* lpGUID, LPSTR  lpDriverDescription, LPSTR  lpDriverName, LPVOID lpContext, HMONITOR hm) -> BOOL
		{
			uint64_t guid[2]{ 0 };
			if (lpGUID)
				memcpy(guid, lpGUID, sizeof(GUID));

			printf("DirectDrawEnumerateExA: %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\" monitor %p\n", guid[0], guid[1], lpDriverName, lpDriverDescription, hm);
#if 0
			IDirectDraw* pdd = nullptr;
			HRESULT r = DirectDrawCreate(lpGUID, &pdd, nullptr);
			printf("%d (%s) DirectDrawCreate %p\n", r, ddResultToStr(r).c_str(), pdd);
			if (pdd)
			{
				printModesDevices(pdd);
				VerboseRelease(pdd, "IDirectDraw");
			}
#endif
			return TRUE;
		}, nullptr, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
	printf("%d (%s) DirectDrawEnumerateExA\n", r, ddResultToStr(r).c_str());

	IDirectDraw* pdd = nullptr;
	IDirectDraw7* pdd7 = nullptr;

	r = DirectDrawCreate(nullptr, &pdd, nullptr);
	printf("%d (%s) DirectDrawCreate %p\n", r, ddResultToStr(r).c_str(), pdd);
	if (!pdd || r != DD_OK)
		return -1;

	r = DirectDrawCreateEx(nullptr, (void**)&pdd7, IID_IDirectDraw7, nullptr);
	printf("%d (%s) DirectDrawCreateEx(DD7) %p\n", r, ddResultToStr(r).c_str(), pdd7);


	printModesDevices(pdd, pdd7);

	auto enumDevices = [](GUID FAR* lpGUID, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC descHW, LPD3DDEVICEDESC descHEL, LPVOID user) -> HRESULT
		{
			uint64_t guid[2]{ 0 };
			if (lpGUID)
				memcpy(guid, lpGUID, sizeof(GUID));

			printf("EnumDevices: %.16" PRIX64 "%.16" PRIX64 " \"%s\" \"%s\"\n", guid[0], guid[1], lpDeviceName, lpDeviceDescription);
			printD3DDCaps("HW", *descHW);
			printD3DDCaps("HEL", *descHEL);
			(*(int*)user)++;
			return DDENUMRET_OK;
		};

	int count = 0;
#if(DIRECT3D_VERSION >= 0x0600)
	printf("DX6:\n");
	IDirect3D3* pd3d3 = nullptr;
	r = pdd->QueryInterface(IID_IDirect3D3, (void**)&pd3d3);
	printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirect3D3) %p\n", r, ddResultToStr(r).c_str(), pd3d3);
	if (pd3d3)
	{
		r = pd3d3->EnumDevices(enumDevices, &count);
		printf("%d (%s) D3D3::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);
		VerboseRelease(pd3d3, "IDirect3D3");
	}
#endif
#if(DIRECT3D_VERSION >= 0x0500)
	printf("DX5:\n");
	IDirect3D2* pd3d = nullptr;
	r = pdd->QueryInterface(IID_IDirect3D2, (void**)&pd3d);
	printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirect3D2) %p\n", r, ddResultToStr(r).c_str(), pd3d);
#else
	printf("DX2:\n");
	IDirect3D* pd3d = nullptr;
	r = pdd->QueryInterface(IID_IDirect3D, (void**)&pd3d);
	printf("%d (%s) IDirectDraw::QueryInterface(IID_IDirect3D) %p\n", r, ddResultToStr(r).c_str(), pd3d);
#endif
	if (!pd3d || r != DD_OK)
		return -1;

	count = 0;
	r = pd3d->EnumDevices(enumDevices, &count);
	printf("%d (%s) D3D::EnumDevices count %d\n", r, ddResultToStr(r).c_str(), count);

	VerboseRelease(pd3d, "IDirect3D");
	VerboseRelease(pdd, "IDirectDraw");

	return 0;
}
