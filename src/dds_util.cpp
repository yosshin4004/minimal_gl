/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "dds_parser.h"
#include "dds_util.h"


static const DdsGlFormat s_tblGlFormats[] = {
	/* DxgiFormat_Unknown */				{0,0,0},
	/* DxgiFormat_R32G32B32A32Typeless */	{0,0,0},
	/* DxgiFormat_R32G32B32A32Float */		{GL_RGBA32F,		GL_RGBA,			GL_FLOAT},
	/* DxgiFormat_R32G32B32A32Uint */		{GL_RGBA32UI,		GL_RGBA_INTEGER,	GL_UNSIGNED_INT},
	/* DxgiFormat_R32G32B32A32Sint */		{GL_RGBA32I,		GL_RGBA_INTEGER,	GL_INT},

	/* DxgiFormat_R32G32B32Typeless */		{0,0,0},
	/* DxgiFormat_R32G32B32Float */			{GL_RGB32F,			GL_RGB,				GL_FLOAT},
	/* DxgiFormat_R32G32B32Uint */			{GL_RGB32UI,		GL_RGB_INTEGER,		GL_UNSIGNED_INT},
	/* DxgiFormat_R32G32B32Sint */			{GL_RGB32I,			GL_RGB_INTEGER,		GL_INT},

	/* DxgiFormat_R16G16B16A16Typeless */	{0,0,0},
	/* DxgiFormat_R16G16B16A16Float */		{GL_RGBA16F,		GL_RGBA,			GL_HALF_FLOAT},
	/* DxgiFormat_R16G16B16A16Unorm */		{GL_RGBA16,			GL_RGBA,			GL_UNSIGNED_SHORT},
	/* DxgiFormat_R16G16B16A16Uint */		{GL_RGBA16UI,		GL_RGBA_INTEGER,	GL_UNSIGNED_SHORT},
	/* DxgiFormat_R16G16B16A16Snorm */		{GL_RGBA16_SNORM,	GL_RGBA,			GL_SHORT},
	/* DxgiFormat_R16G16B16A16Sint */		{GL_RGBA16I,		GL_RGBA_INTEGER,	GL_SHORT},

	/* DxgiFormat_R32G32Typeless */			{0,0,0},
	/* DxgiFormat_R32G32Float */			{GL_RG32F,			GL_RG,				GL_FLOAT},
	/* DxgiFormat_R32G32Uint */				{GL_RG32UI,			GL_RG_INTEGER,		GL_UNSIGNED_INT},
	/* DxgiFormat_R32G32Sint */				{GL_RG32I,			GL_RG_INTEGER,		GL_INT},

	/* DxgiFormat_R32G8X24Typeless */		{0,0,0},
	/* DxgiFormat_D32FloatS8X24Uint */		{0,0,0},
	/* DxgiFormat_R32FloatX8X24Typeless */	{0,0,0},
	/* DxgiFormat_X32TypelessG8X24Uint */	{0,0,0},

	/* DxgiFormat_R10G10B10A2Typeless */	{0,0,0},
	/* DxgiFormat_R10G10B10A2Unorm */		{GL_RGB10_A2,		GL_RGBA,			GL_UNSIGNED_INT_2_10_10_10_REV},
	/* DxgiFormat_R10G10B10A2Uint */		{GL_RGB10_A2UI,		GL_RGBA_INTEGER,	GL_UNSIGNED_INT_2_10_10_10_REV},

	/* DxgiFormat_R11G11B10Float */			{GL_R11F_G11F_B10F,	GL_RGB,				GL_UNSIGNED_INT_10F_11F_11F_REV},

	/* DxgiFormat_R8G8B8A8Typeless */		{0,0,0},
	/* DxgiFormat_R8G8B8A8Unorm */			{GL_RGBA8,			GL_RGBA,			GL_UNSIGNED_BYTE},
	/* DxgiFormat_R8G8B8A8UnormSrgb */		{GL_SRGB8_ALPHA8,	GL_RGBA,			GL_UNSIGNED_BYTE},
	/* DxgiFormat_R8G8B8A8Uint */			{GL_RGBA8UI,		GL_RGBA_INTEGER,	GL_UNSIGNED_BYTE},
	/* DxgiFormat_R8G8B8A8Snorm */			{GL_RGBA8_SNORM,	GL_RGBA,			GL_BYTE},
	/* DxgiFormat_R8G8B8A8Sint */			{GL_RGBA8I,			GL_RGBA_INTEGER,	GL_BYTE},

	/* DxgiFormat_R16G16Typeless */			{0,0,0},
	/* DxgiFormat_R16G16Float */			{GL_RG16F,			GL_RG,				GL_HALF_FLOAT},
	/* DxgiFormat_R16G16Unorm */			{GL_RG16,			GL_RG,				GL_UNSIGNED_SHORT},
	/* DxgiFormat_R16G16Uint */				{GL_RG16UI,			GL_RG_INTEGER,		GL_UNSIGNED_SHORT},
	/* DxgiFormat_R16G16Snorm */			{GL_RG16_SNORM,		GL_RG,				GL_SHORT},
	/* DxgiFormat_R16G16Sint */				{GL_RG16I,			GL_RG_INTEGER,		GL_SHORT},

	/* DxgiFormat_R32Typeless */			{0,0,0},
	/* DxgiFormat_D32Float */				{0,0,0},
	/* DxgiFormat_R32Float */				{GL_R32F,			GL_RED,				GL_FLOAT},
	/* DxgiFormat_R32Uint */				{GL_R32UI,			GL_RED_INTEGER,		GL_UNSIGNED_INT},
	/* DxgiFormat_R32Sint */				{GL_R32I,			GL_RED_INTEGER,		GL_INT},

	/* DxgiFormat_R24G8Typeless */			{0,0,0},
	/* DxgiFormat_D24UnormS8Uint */			{0,0,0},
	/* DxgiFormat_R24UnormX8Typeless */		{0,0,0},
	/* DxgiFormat_X24TypelessG8Uint */		{0,0,0},

	/* DxgiFormat_R8G8Typeless */			{0,0,0},
	/* DxgiFormat_R8G8Unorm */				{GL_RG8,			GL_RG,				GL_UNSIGNED_BYTE},
	/* DxgiFormat_R8G8Uint */				{GL_RG8UI,			GL_RG_INTEGER,		GL_UNSIGNED_BYTE},
	/* DxgiFormat_R8G8Snorm */				{GL_RG8_SNORM,		GL_RG,				GL_BYTE},
	/* DxgiFormat_R8G8Sint */				{GL_RG8I,			GL_RG_INTEGER,		GL_BYTE},

	/* DxgiFormat_R16Typeless */			{0,0,0},
	/* DxgiFormat_R16Float */				{GL_R16F,			GL_RED,				GL_HALF_FLOAT},
	/* DxgiFormat_D16Unorm */				{0,0,0},
	/* DxgiFormat_R16Unorm */				{GL_R16,			GL_RED,				GL_UNSIGNED_SHORT},
	/* DxgiFormat_R16Uint */				{GL_R16UI,			GL_RED_INTEGER,		GL_UNSIGNED_SHORT},
	/* DxgiFormat_R16Snorm */				{GL_R16_SNORM,		GL_RED,				GL_UNSIGNED_SHORT},
	/* DxgiFormat_R16Sint */				{GL_R16I,			GL_RED_INTEGER,		GL_SHORT},

	/* DxgiFormat_R8Typeless */				{0,0,0},
	/* DxgiFormat_R8Unorm */				{GL_R8,				GL_RED,				GL_UNSIGNED_BYTE},
	/* DxgiFormat_R8Uint */					{GL_R8UI,			GL_RED_INTEGER,		GL_UNSIGNED_BYTE},
	/* DxgiFormat_R8Snorm */				{GL_R8_SNORM,		GL_RED,				GL_BYTE},
	/* DxgiFormat_R8Sint */					{GL_R8I,			GL_RED_INTEGER,		GL_BYTE},
	/* DxgiFormat_A8Unorm */				{GL_ALPHA8,			GL_RED,				GL_UNSIGNED_BYTE},
	/* DxgiFormat_R1Unorm */				{0,0,0},

	/* DxgiFormat_R9G9B9E5SharedExp */		{GL_RGB9_E5,		GL_RGB,				GL_UNSIGNED_INT_5_9_9_9_REV},

	/* DxgiFormat_R8G8B8G8Unorm */			{0,0,0},
	/* DxgiFormat_G8R8G8B8Unorm */			{0,0,0},

	/* DxgiFormat_Bc1Typeless */			{0,0,0},
	/* DxgiFormat_Bc1Unorm */				{GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0, 0},
	/* DxgiFormat_Bc1UnormSrgb */			{GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 0, 0},

	/* DxgiFormat_Bc2Typeless */			{0,0,0},
	/* DxgiFormat_Bc2Unorm */				{GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, 0},
	/* DxgiFormat_Bc2UnormSrgb */			{GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 0, 0},

	/* DxgiFormat_Bc3Typeless */			{0,0,0},
	/* DxgiFormat_Bc3Unorm */				{GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0, 0},
	/* DxgiFormat_Bc3UnormSrgb */			{GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 0, 0},

	/* DxgiFormat_Bc4Typeless */			{0,0,0},
	/* DxgiFormat_Bc4Unorm */				{GL_COMPRESSED_LUMINANCE_LATC1_EXT, 0, 0},
	/* DxgiFormat_Bc4Snorm */				{GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT, 0, 0},

	/* DxgiFormat_Bc5Typeless */			{0,0,0},
	/* DxgiFormat_Bc5Unorm */				{GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT, 0, 0},
	/* DxgiFormat_Bc5Snorm */				{GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT, 0, 0},

	/* DxgiFormat_B5G6R5Unorm */			{GL_RGB565,			GL_RGB,				GL_UNSIGNED_SHORT_5_6_5},
	/* DxgiFormat_B5G5R5A1Unorm */			{GL_RGB5_A1,		GL_BGRA,			GL_UNSIGNED_SHORT_1_5_5_5_REV},
	/* DxgiFormat_B8G8R8A8Unorm */			{GL_RGBA8,			GL_BGRA,			GL_UNSIGNED_BYTE},
	/* DxgiFormat_B8G8R8X8Unorm */			{GL_RGBA8,			GL_BGRA,			GL_UNSIGNED_BYTE},
	/* DxgiFormat_R10G10B10XrBiasA2Unorm */	{0,0,0},
	/* DxgiFormat_B8G8R8A8Typeless */		{0,0,0},
	/* DxgiFormat_B8G8R8A8UnormSrgb */		{GL_SRGB8_ALPHA8,	GL_BGRA,			GL_UNSIGNED_BYTE},
	/* DxgiFormat_B8G8R8X8Typeless */		{0,0,0},
	/* DxgiFormat_B8G8R8X8UnormSrgb */		{GL_SRGB8_ALPHA8,	GL_BGRA,			GL_UNSIGNED_BYTE},

	/* DxgiFormat_Bc6hTypeless */			{0,0,0},
	/* DxgiFormat_Bc6hUf16 */				{GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, 0, 0},
	/* DxgiFormat_Bc6hSf16 */				{GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, 0, 0},

	/* DxgiFormat_Bc7Typeless */			{0,0,0},
	/* DxgiFormat_Bc7Unorm */				{GL_COMPRESSED_RGBA_BPTC_UNORM, 0, 0},
	/* DxgiFormat_Bc7UnormSrgb */			{GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM, 0, 0},

	/* DxgiFormat_Ayuv */					{0,0,0},
	/* DxgiFormat_Y410 */					{0,0,0},
	/* DxgiFormat_Y416 */					{0,0,0},
	/* DxgiFormat_Nv12 */					{0,0,0},
	/* DxgiFormat_P010 */					{0,0,0},
	/* DxgiFormat_P016 */					{0,0,0},
	/* DxgiFormat_420Opaque */				{0,0,0},
	/* DxgiFormat_Yuy2 */					{0,0,0},
	/* DxgiFormat_Y210 */					{0,0,0},
	/* DxgiFormat_Y216 */					{0,0,0},
	/* DxgiFormat_Nv11 */					{0,0,0},
	/* DxgiFormat_Ai44 */					{0,0,0},
	/* DxgiFormat_Ia44 */					{0,0,0},
	/* DxgiFormat_P8 */						{0,0,0},
	/* DxgiFormat_A8P8 */					{0,0,0},
	/* DxgiFormat_B4G4R4A4Unorm */			{GL_RGBA4,			GL_BGRA,			GL_UNSIGNED_SHORT_4_4_4_4_REV},
};
DdsGlFormat
DdsDxgiFormatToGlFormat(
	DxgiFormat format
){
	assert(format < SIZE_OF_ARRAY(s_tblGlFormats));
	return s_tblGlFormats[format];
}


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


