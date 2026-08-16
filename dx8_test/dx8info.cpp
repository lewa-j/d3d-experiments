
#include <stdio.h>
#include "d3d8.h"

#define Log printf

static const char *D3DDEVTYPE_to_str(D3DDEVTYPE dt)
{
	switch(dt)
	{
	case D3DDEVTYPE_HAL:
		return "HAL";
	case D3DDEVTYPE_REF:
		return "REF";
	case D3DDEVTYPE_SW:
		return "SW";
	default:
		return "Unknown";
	}
}

static void printCmpCaps(const char *title, DWORD c)
{
	Log("%s %X(", title, c);
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
	if (c & ~0x000000FFL)
		Log("Unknown flags %X", c & ~0x000000FFL);
	Log(")\n");
}

static void printBlendCaps(const char *title, DWORD c)
{
	Log("%s %X(", title, c);
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
	if (c & ~0x00001FFFL)
		Log("Unknown flags %X", c & ~0x00001FFFL);
	Log(")\n");
}

static void printTextureFilterCaps(const char *title, DWORD c)
{
	Log("%s %X(", title, c);
#define X(name) if (c & D3DPTFILTERCAPS_##name) printf(#name " ");
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
#undef X
	if (c & ~0x1F030700L)
		Log("Unknown flags %X", c & ~0x1F030700L);
	Log(")\n");
}

static void printTextureAddressCaps(const char *title, DWORD c)
{
	Log("%s %X(", title, c);
#define X(name) if (c & D3DPTADDRESSCAPS_##name) printf(#name " ");
	X(WRAP)
	X(MIRROR)
	X(CLAMP)
	X(BORDER)
	X(INDEPENDENTUV)
	X(MIRRORONCE)
#undef X
	if (c & ~0x0000003FL)
		Log("Unknown flags %X", c & ~0x0000003FL);
	Log(")\n");
}

void printD3D8Info(const D3DCAPS8 &c)
{
	Log("	DeviceType %d(%s) AdapterOrdinal %d\n",
		c.DeviceType, D3DDEVTYPE_to_str(c.DeviceType), c.AdapterOrdinal);

	Log("	DD7 Caps %X(", c.Caps);
	if (c.Caps & D3DCAPS_READ_SCANLINE)
		Log("READ_SCANLINE");
	if (c.Caps & ~D3DCAPS_READ_SCANLINE)
		Log(" Unknown flags %X", c.Caps & ~D3DCAPS_READ_SCANLINE);
	Log(") Caps2 %X(", c.Caps2);
#define X(name) if (c.Caps2 & D3DCAPS2_##name) printf(#name " ");
	X(NO2DDURING3DSCENE)
	X(FULLSCREENGAMMA)
	X(CANRENDERWINDOWED)
	X(CANCALIBRATEGAMMA)
	X(RESERVED)
	X(CANMANAGERESOURCE)
	X(DYNAMICTEXTURES)
#undef X
	if (c.Caps2 & ~0x321A0002L)
		Log("Unknown flags %X", c.Caps2 & ~0x321A0002L);
	Log(") Caps3 %X(", c.Caps3);
	if (c.Caps3 & D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD)
		Log("ALPHA_FULLSCREEN_FLIP_OR_DISCARD");
	if (c.Caps3 & ~D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD)
		Log(" Unknown flags %X", c.Caps3 & ~D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD);
	Log(")\n");

	Log("	PresentationIntervals %X(", c.PresentationIntervals);
#define X(name) if (c.PresentationIntervals & D3DPRESENT_INTERVAL_##name) printf(#name " ");
	X(DEFAULT)
	X(ONE)
	X(TWO)
	X(THREE)
	X(FOUR)
	X(IMMEDIATE)
#undef X
	if (c.PresentationIntervals & ~0x8000000FL)
		Log("Unknown flags %X", c.PresentationIntervals & ~0x8000000FL);
	Log(")\n");

	Log("	Cursor Caps %X\n", c.CursorCaps);
	Log("	DevCaps %X(", c.DevCaps);
#define X(name) if (c.DevCaps & D3DDEVCAPS_##name) printf(#name " ");
	X(EXECUTESYSTEMMEMORY)
	X(EXECUTEVIDEOMEMORY)
	X(TLVERTEXSYSTEMMEMORY)
	X(TLVERTEXVIDEOMEMORY)
	X(TEXTURESYSTEMMEMORY)
	X(TEXTUREVIDEOMEMORY)
	X(DRAWPRIMTLVERTEX)
	X(CANRENDERAFTERFLIP)
	X(TEXTURENONLOCALVIDMEM)
	X(DRAWPRIMITIVES2)
	X(SEPARATETEXTUREMEMORIES)
	X(DRAWPRIMITIVES2EX)
	X(HWTRANSFORMANDLIGHT)
	X(CANBLTSYSTONONLOCAL)
	X(HWRASTERIZATION)
	X(PUREDEVICE)
	X(QUINTICRTPATCHES)
	X(RTPATCHES)
	X(RTPATCHHANDLEZERO)
	X(NPATCHES)
#undef X
	if (c.DevCaps & ~0x01FBFFF0L)
		Log("Unknown flags %X", c.DevCaps & ~0x01FBFFF0L);
	Log(")\n");

	Log("	PrimitiveMiscCaps %X(", c.PrimitiveMiscCaps);
#define X(name) if (c.PrimitiveMiscCaps & D3DPMISCCAPS_##name) printf(#name " ");
	X(MASKZ)
	X(LINEPATTERNREP)
	X(CULLNONE)
	X(CULLCW)
	X(CULLCCW)
	X(COLORWRITEENABLE)
	X(CLIPPLANESCALEDPOINTS)
	X(CLIPTLVERTS)
	X(TSSARGTEMP)
	X(BLENDOP)
	X(NULLREFERENCE)
#undef X
	if (c.PrimitiveMiscCaps & ~0x00001FF6L)
		Log("Unknown flags %X", c.PrimitiveMiscCaps & ~0x00001FF6L);
	Log(")\n");

	Log("	RasterCaps %X(", c.RasterCaps);
#define X(name) if (c.RasterCaps & D3DPRASTERCAPS_##name) printf(#name " ");
	X(DITHER)
	//? 0x2
	//? 0x4
	X(PAT)
	X(ZTEST)
	X(FOGVERTEX)
	X(FOGTABLE)
	X(ANTIALIASEDGES)
	X(MIPMAPLODBIAS)
	X(ZBIAS)
	X(ZBUFFERLESSHSR)
	X(FOGRANGE)
	X(ANISOTROPY)
	X(WBUFFER)
	X(WFOG)
	X(ZFOG)
	X(COLORPERSPECTIVE)
	X(STRETCHBLTMULTISAMPLE)
#undef X
	if (c.RasterCaps & ~0x00F7F199L)
		Log("Unknown flags %X", c.RasterCaps & ~0x00F7F199L);
	Log(")\n");

	printCmpCaps("	ZCmpCaps", c.ZCmpCaps);
	printBlendCaps("	SrcBlendCaps", c.SrcBlendCaps);
	printBlendCaps("	DestBlendCaps", c.DestBlendCaps);
	printCmpCaps("	AlphaCmpCaps", c.AlphaCmpCaps);

	Log("	ShadeCaps %X(", c.ShadeCaps);
#define X(name) if (c.ShadeCaps & D3DPSHADECAPS_##name) printf(#name " ");
	X(COLORGOURAUDRGB)
	X(SPECULARGOURAUDRGB)
	X(ALPHAGOURAUDBLEND)
	X(FOGGOURAUD)
#undef X
	if (c.ShadeCaps & ~0x00084208L)
		Log("Unknown flags %X", c.ShadeCaps & ~0x00084208L);
	Log(")\n");

	Log("	TextureCaps %X(", c.TextureCaps);
#define X(name) if (c.TextureCaps & D3DPTEXTURECAPS_##name) printf(#name " ");
	X(PERSPECTIVE)
	X(POW2)
	X(ALPHA)
	X(SQUAREONLY)
	X(TEXREPEATNOTSCALEDBYSIZE)
	X(ALPHAPALETTE)
	X(NONPOW2CONDITIONAL)
	X(PROJECTED)
	X(CUBEMAP)
	X(VOLUMEMAP)
	X(MIPMAP)
	X(MIPVOLUMEMAP)
	X(MIPCUBEMAP)
	X(CUBEMAP_POW2)
	X(VOLUMEMAP_POW2)
#undef X
	if (c.TextureCaps & ~0x0007EDE7L)
		Log("Unknown flags %X", c.TextureCaps & ~0x0007EDE7L);
	Log(")\n");

	printTextureFilterCaps("	TextureFilterCaps", c.TextureFilterCaps);
	printTextureFilterCaps("	CubeTextureFilterCaps", c.CubeTextureFilterCaps);
	printTextureFilterCaps("	VolumeTextureFilterCaps", c.VolumeTextureFilterCaps);

	printTextureAddressCaps("	TextureAddressCaps", c.TextureAddressCaps);
	printTextureAddressCaps("	VolumeTextureAddressCaps", c.VolumeTextureAddressCaps);

	Log("	LineCaps %X(", c.LineCaps);
#define X(name) if (c.LineCaps & D3DLINECAPS_##name) printf(#name " ");
	X(TEXTURE)
	X(ZTEST)
	X(BLEND)
	X(ALPHACMP)
	X(FOG)
#undef X
	if (c.LineCaps & ~0x0000001FL)
		Log("Unknown flags %X", c.LineCaps & ~0x0000001FL);
	Log(")\n");

	Log("	MaxTextureSize %dx%dx%d\n",
		c.MaxTextureWidth, c.MaxTextureHeight, c.MaxVolumeExtent);

	Log("	MaxTextureRepeat %d MaxTextureAspectRatio %d MaxAnisotropy %d\n",
		c.MaxTextureRepeat, c.MaxTextureAspectRatio, c.MaxAnisotropy);

	Log("	MaxVertexW %f ExtentsAdjust %f MaxPointSize %f\n",
		c.MaxVertexW, c.ExtentsAdjust, c.MaxPointSize);

	Log("	GuardBand %g %g %g %g\n",
		c.GuardBandLeft, c.GuardBandTop, c.GuardBandRight, c.GuardBandBottom);

	Log("	StencilCaps %X(", c.StencilCaps);
#define X(name) if (c.StencilCaps & D3DSTENCILCAPS_##name) printf(#name " ");
	X(KEEP)
	X(ZERO)
	X(REPLACE)
	X(INCRSAT)
	X(DECRSAT)
	X(INVERT)
	X(INCR)
	X(DECR)
#undef X
	if (c.StencilCaps & ~0x000000FFL)
		Log("Unknown flags %X", c.StencilCaps & ~0x000000FFL);
	Log(")\n");

	Log("	FVFCaps %X(Tex coords %d, ", c.FVFCaps, c.FVFCaps & D3DFVFCAPS_TEXCOORDCOUNTMASK);
#define X(name) if (c.FVFCaps & D3DFVFCAPS_##name) printf(#name " ");
	X(DONOTSTRIPELEMENTS)
	X(PSIZE)
#undef X
	if (c.FVFCaps & ~0x0018FFFFL)
		Log("Unknown flags %X", c.FVFCaps & ~0x0018FFFFL);
	Log(")\n");

	Log("	TextureOpCaps %X(", c.TextureOpCaps);
#define X(name) if (c.TextureOpCaps & D3DTEXOPCAPS_##name) printf(#name " ");
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
	X(MULTIPLYADD)
	X(LERP)
#undef X
	if (c.TextureOpCaps & ~0x03FFFFFFL)
		Log("Unknown flags %X", c.TextureOpCaps & ~0x03FFFFFFL);
	Log(")\n");

	Log("	MaxTextureBlendStages %d MaxSimultaneousTextures %d\n",
		c.MaxTextureBlendStages, c.MaxSimultaneousTextures);

	Log("	VertexProcessingCaps %X(", c.VertexProcessingCaps);
#define X(name) if (c.VertexProcessingCaps & D3DVTXPCAPS_##name) printf(#name " ");
	X(TEXGEN)
	X(MATERIALSOURCE7)
	X(DIRECTIONALLIGHTS)
	X(POSITIONALLIGHTS)
	X(LOCALVIEWER)
	X(TWEENING)
	X(NO_VSDT_UBYTE4)
#undef X
	if (c.VertexProcessingCaps & ~0x000000FBL)
		Log("Unknown flags %X", c.VertexProcessingCaps & ~0x000000FBL);
	Log(")\n");

	Log("	MaxActiveLights %d MaxUserClipPlanes %d\n"
		"	MaxVertexBlendMatrices %d MaxVertexBlendMatrixIndex %d\n",
		c.MaxActiveLights, c.MaxUserClipPlanes, c.MaxVertexBlendMatrices, c.MaxVertexBlendMatrixIndex);

	Log("	MaxPrimitiveCount %d MaxVertexIndex %d\n"
		"	MaxStreams %d MaxStreamStride %d\n",
		c.MaxPrimitiveCount, c.MaxVertexIndex, c.MaxStreams, c.MaxStreamStride);

	Log("	VertexShaderVersion 0x%X MaxVertexShaderConst %d\n",
		c.VertexShaderVersion, c.MaxVertexShaderConst);

	Log("	PixelShaderVersion 0x%X MaxPixelShaderValue %g\n",
		c.PixelShaderVersion, c.MaxPixelShaderValue);
}
