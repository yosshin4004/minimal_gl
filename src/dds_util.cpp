/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "dds_util.h"

typedef struct {
	uint32_t	dwSize;
	uint32_t	dwFlags;
	uint32_t	dwFourCC;
	uint32_t	dwRGBBitCount;
	uint32_t	dwRBitMask;
	uint32_t	dwGBitMask;
	uint32_t	dwBBitMask;
	uint32_t	dwABitMask;
} DdsPixelFormat;

typedef struct {
	uint32_t		dwMagic;
	uint32_t		dwSize;
	uint32_t		dwFlags;
	uint32_t		dwHeight;
	uint32_t		dwWidth;
	uint32_t		dwPitchOrLinearSize;
	uint32_t		dwDepth;
	uint32_t		dwMipMapCount;
	uint32_t		dwReserved1[11];
	DdsPixelFormat	ddspf;
	uint32_t		dwCaps;
	uint32_t		dwCaps2;
	uint32_t		dwCaps3;
	uint32_t		dwCaps4;
	uint32_t		dwReserved2;
} DdsHeader;

typedef enum {
	Ddsd_Caps			= 0x00000001,
	Ddsd_Height			= 0x00000002,
	Ddsd_Width			= 0x00000004,
	Ddsd_Pitch			= 0x00000008,
	Ddsd_PixelFormat	= 0x00001000,
	Ddsd_MipmapCount	= 0x00020000,
	Ddsd_LinearSize		= 0x00080000,
	Ddsd_Depth			= 0x00800000
} Ddsd;

typedef enum {
	DdsdFourCC_Dx10				= 'D' | ((int)'X' << 8) | ((int)'1' << 16) | ((int)'0' << 24),
	DdsdFourCC_Dxt1				= 'D' | ((int)'X' << 8) | ((int)'T' << 16) | ((int)'1' << 24),
	DdsdFourCC_Dxt2				= 'D' | ((int)'X' << 8) | ((int)'T' << 16) | ((int)'2' << 24),
	DdsdFourCC_Dxt3				= 'D' | ((int)'X' << 8) | ((int)'T' << 16) | ((int)'3' << 24),
	DdsdFourCC_Dxt4				= 'D' | ((int)'X' << 8) | ((int)'T' << 16) | ((int)'4' << 24),
	DdsdFourCC_Dxt5				= 'D' | ((int)'X' << 8) | ((int)'T' << 16) | ((int)'5' << 24),

	DdsdFourCC_Bc4Unorm			= 'B' | ((int)'C' << 8) | ((int)'4' << 16) | ((int)'U' << 24),
	DdsdFourCC_Bc4Snorm			= 'B' | ((int)'C' << 8) | ((int)'4' << 16) | ((int)'S' << 24),
	DdsdFourCC_Bc5Unorm			= 'B' | ((int)'C' << 8) | ((int)'5' << 16) | ((int)'U' << 24),
	DdsdFourCC_Bc5Snorm			= 'B' | ((int)'C' << 8) | ((int)'5' << 16) | ((int)'S' << 24),

	DdsdFourCC_A16B16G16R16		= 0x00000024,
	DdsdFourCC_Q16W16V16U16		= 0x0000006e,
	DdsdFourCC_R16F				= 0x0000006f,
	DdsdFourCC_G16R16F			= 0x00000070,
	DdsdFourCC_A16B16G16R16F	= 0x00000071,
	DdsdFourCC_R32F				= 0x00000072,
	DdsdFourCC_G32R32F			= 0x00000073,
	DdsdFourCC_A32B32G32R32F	= 0x00000074,
	DdsdFourCC_CxV8U8			= 0x00000075,
	DdsdFourCC_Q8W8V8U8			= 0x0000003f,

	DdsdFourCC_3dcAti2			= 'A' | ((int)'T' << 8) | ((int)'I' << 16) | ((int)'2' << 24)
} DdsdFourCC;

typedef enum {
	Ddpf_AlphaPixels	= 0x00000001,
	Ddpf_Alpha			= 0x00000002,
	Ddpf_FourCC			= 0x00000004,
	Ddpf_Rgb			= 0x00000040,
	Ddpf_Yuv			= 0x00000200,
	Ddpf_Luminance		= 0x00020000,
	Ddpf_BumpDuDy		= 0x00080000
} Ddpf;

typedef enum {
	DdsCaps_Complex	= 0x00000008,
	DdsCaps_Texture	= 0x00001000,
	DdsCaps_Mipmap	= 0x00400000
} DdsCaps;

typedef enum {
	DdsCaps2_Cubemap			= 0x00000200,
	DdsCaps2_CubemapPositiveX	= 0x00000400,
	DdsCaps2_CubemapNegativeX	= 0x00000800,
	DdsCaps2_CubemapPositiveY	= 0x00001000,
	DdsCaps2_CubemapNegativeY	= 0x00002000,
	DdsCaps2_CubemapPositiveZ	= 0x00004000,
	DdsCaps2_CubemapNegativeZ	= 0x00008000,
	DdsCaps2_Volume				= 0x00200000
} DdsCaps2;


bool SerializeAsFp32RgbaCubemapDds(
	const char *fileName,
	float *(data[6]),
	int width,
	int height
){
	FILE *file = fopen(fileName, "wb");
	if (file == NULL) return false;

	/* ヘッダの書き出し */
	{
		uint32_t	pfFlags = Ddpf_FourCC;
		uint32_t	fourCC = DdsdFourCC_A32B32G32R32F;
		uint32_t	RGBBitCount = 0;
		uint32_t	RBitMask = 0;
		uint32_t	GBitMask = 0;
		uint32_t	BBitMask = 0;
		uint32_t	ABitMask = 0;
		uint32_t	caps = DdsCaps_Texture | DdsCaps_Complex;
		uint32_t	caps2 =
						DdsCaps2_Cubemap
					|	DdsCaps2_CubemapPositiveX
					|	DdsCaps2_CubemapNegativeX
					|	DdsCaps2_CubemapPositiveY
					|	DdsCaps2_CubemapNegativeY
					|	DdsCaps2_CubemapPositiveZ
					|	DdsCaps2_CubemapNegativeZ
					;

		DdsHeader ddsHeader = {
			/* +0x00 : uint32_t	dwMagic */				0x20534444,
			/* +0x04 : uint32_t	dwSize */				124,
			/* +0x08 : uint32_t	dwFlags */				(	Ddsd_Caps
														|	Ddsd_Height
														|	Ddsd_Width
														|	Ddsd_PixelFormat
														|	Ddsd_MipmapCount
														),
			/* +0x0C : uint32_t	dwHeight */				(uint32_t)height,
			/* +0x10 : uint32_t	dwWidth */				(uint32_t)width,
			/* +0x14 : uint32_t	dwPitchOrLinearSize */	0,
			/* +0x18 : uint32_t	dwDepth */				1,
			/* +0x1C : uint32_t	dwMipMapCount */		1,
			/* +0x20 : uint32_t	dwReserved1[11] */		{0,0,0,0, 0,0,0,0, 0,0,0},
														{
			/* +0x4C : uint32_t	ddspf.dwSize */				32,
			/* +0x50 : uint32_t	ddspf.dwFlags */			pfFlags,
			/* +0x54 : uint32_t	ddspf.dwFourCC */			fourCC,
			/* +0x58 : uint32_t	ddspf.dwRGBBitCount */		RGBBitCount,
			/* +0x5C : uint32_t	ddspf.dwRBitMask */			RBitMask,
			/* +0x60 : uint32_t	ddspf.dwGBitMask */			GBitMask,
			/* +0x64 : uint32_t	ddspf.dwBBitMask */			BBitMask,
			/* +0x68 : uint32_t	ddspf.dwABitMask */			ABitMask
														},
			/* +0x6C : uint32_t	dwCaps */				caps,
			/* +0x70 : uint32_t	dwCaps2 */				caps2,
			/* +0x74 : uint32_t	dwCaps3 */				0,
			/* +0x78 : uint32_t	dwCaps4 */				0,
			/* +0x7C : uint32_t	dwReserved2 */			0
		};
		fwrite(&ddsHeader, 1, sizeof(ddsHeader), file);
	}

	/* イメージデータの書き出し */
	for (int iFace = 0; iFace < 6; iFace++) {
		fwrite(data[iFace], 1, width * height * sizeof(float) * 4, file);
	}

	fclose(file);

	return true;
}


