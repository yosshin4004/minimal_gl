/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <stdbool.h>
#include <stdint.h>
#include "dds_parser.h"

#undef	SIZE_OF_ARRAY
#define	SIZE_OF_ARRAY(a) (sizeof(a) / sizeof(a[0]))

static int IntMax(int x, int y){
	return (x > y)? x: y;
}
static int IntCeilAlign(int x, int align){
	return (x + (align - 1)) & ~(align - 1);
}

static const int s_tblNumBitsPerPixel[] = {
	/* DxgiFormat_Unknown */				0,
	/* DxgiFormat_R32G32B32A32Typeless */	128,
	/* DxgiFormat_R32G32B32A32Float */		128,
	/* DxgiFormat_R32G32B32A32Uint */		128,
	/* DxgiFormat_R32G32B32A32Sint */		128,

	/* DxgiFormat_R32G32B32Typeless */		96,
	/* DxgiFormat_R32G32B32Float */			96,
	/* DxgiFormat_R32G32B32Uint */			96,
	/* DxgiFormat_R32G32B32Sint */			96,

	/* DxgiFormat_R16G16B16A16Typeless */	64,
	/* DxgiFormat_R16G16B16A16Float */		64,
	/* DxgiFormat_R16G16B16A16Unorm */		64,
	/* DxgiFormat_R16G16B16A16Uint */		64,
	/* DxgiFormat_R16G16B16A16Snorm */		64,
	/* DxgiFormat_R16G16B16A16Sint */		64,

	/* DxgiFormat_R32G32Typeless */			64,
	/* DxgiFormat_R32G32Float */			64,
	/* DxgiFormat_R32G32Uint */				64,
	/* DxgiFormat_R32G32Sint */				64,

	/* DxgiFormat_R32G8X24Typeless */		64,
	/* DxgiFormat_D32FloatS8X24Uint */		64,
	/* DxgiFormat_R32FloatX8X24Typeless */	64,
	/* DxgiFormat_X32TypelessG8X24Uint */	64,

	/* DxgiFormat_R10G10B10A2Typeless */	32,
	/* DxgiFormat_R10G10B10A2Unorm */		32,
	/* DxgiFormat_R10G10B10A2Uint */		32,

	/* DxgiFormat_R11G11B10Float */			32,

	/* DxgiFormat_R8G8B8A8Typeless */		32,
	/* DxgiFormat_R8G8B8A8Unorm */			32,
	/* DxgiFormat_R8G8B8A8UnormSrgb */		32,
	/* DxgiFormat_R8G8B8A8Uint */			32,
	/* DxgiFormat_R8G8B8A8Snorm */			32,
	/* DxgiFormat_R8G8B8A8Sint */			32,

	/* DxgiFormat_R16G16Typeless */			32,
	/* DxgiFormat_R16G16Float */			32,
	/* DxgiFormat_R16G16Unorm */			32,
	/* DxgiFormat_R16G16Uint */				32,
	/* DxgiFormat_R16G16Snorm */			32,
	/* DxgiFormat_R16G16Sint */				32,

	/* DxgiFormat_R32Typeless */			32,
	/* DxgiFormat_D32Float */				32,
	/* DxgiFormat_R32Float */				32,
	/* DxgiFormat_R32Uint */				32,
	/* DxgiFormat_R32Sint */				32,

	/* DxgiFormat_R24G8Typeless */			32,
	/* DxgiFormat_D24UnormS8Uint */			32,
	/* DxgiFormat_R24UnormX8Typeless */		32,
	/* DxgiFormat_X24TypelessG8Uint */		32,

	/* DxgiFormat_R8G8Typeless */			16,
	/* DxgiFormat_R8G8Unorm */				16,
	/* DxgiFormat_R8G8Uint */				16,
	/* DxgiFormat_R8G8Snorm */				16,
	/* DxgiFormat_R8G8Sint */				16,

	/* DxgiFormat_R16Typeless */			16,
	/* DxgiFormat_R16Float */				16,
	/* DxgiFormat_D16Unorm */				16,
	/* DxgiFormat_R16Unorm */				16,
	/* DxgiFormat_R16Uint */				16,
	/* DxgiFormat_R16Snorm */				16,
	/* DxgiFormat_R16Sint */				16,

	/* DxgiFormat_R8Typeless */				8,
	/* DxgiFormat_R8Unorm */				8,
	/* DxgiFormat_R8Uint */					8,
	/* DxgiFormat_R8Snorm */				8,
	/* DxgiFormat_R8Sint */					8,
	/* DxgiFormat_A8Unorm */				8,
	/* DxgiFormat_R1Unorm */				1,

	/* DxgiFormat_R9G9B9E5SharedExp */		32,

	/* DxgiFormat_R8G8B8G8Unorm */			32,
	/* DxgiFormat_G8R8G8B8Unorm */			32,

	/* DxgiFormat_Bc1Typeless */			4,
	/* DxgiFormat_Bc1Unorm */				4,
	/* DxgiFormat_Bc1UnormSrgb */			4,

	/* DxgiFormat_Bc2Typeless */			8,
	/* DxgiFormat_Bc2Unorm */				8,
	/* DxgiFormat_Bc2UnormSrgb */			8,

	/* DxgiFormat_Bc3Typeless */			8,
	/* DxgiFormat_Bc3Unorm */				8,
	/* DxgiFormat_Bc3UnormSrgb */			8,

	/* DxgiFormat_Bc4Typeless */			4,
	/* DxgiFormat_Bc4Unorm */				4,
	/* DxgiFormat_Bc4Snorm */				4,

	/* DxgiFormat_Bc5Typeless */			8,
	/* DxgiFormat_Bc5Unorm */				8,
	/* DxgiFormat_Bc5Snorm */				8,

	/* DxgiFormat_B5G6R5Unorm */			16,
	/* DxgiFormat_B5G5R5A1Unorm */			16,
	/* DxgiFormat_B8G8R8A8Unorm */			32,
	/* DxgiFormat_B8G8R8X8Unorm */			32,
	/* DxgiFormat_R10G10B10XrBiasA2Unorm */	32,
	/* DxgiFormat_B8G8R8A8Typeless */		32,
	/* DxgiFormat_B8G8R8A8UnormSrgb */		32,
	/* DxgiFormat_B8G8R8X8Typeless */		32,
	/* DxgiFormat_B8G8R8X8UnormSrgb */		32,

	/* DxgiFormat_Bc6hTypeless */			8,
	/* DxgiFormat_Bc6hUf16 */				8,
	/* DxgiFormat_Bc6hSf16 */				8,

	/* DxgiFormat_Bc7Typeless */			8,
	/* DxgiFormat_Bc7Unorm */				8,
	/* DxgiFormat_Bc7UnormSrgb */			8,

	/* DxgiFormat_Ayuv */					32,
	/* DxgiFormat_Y410 */					32,
	/* DxgiFormat_Y416 */					64,
	/* DxgiFormat_Nv12 */					12,
	/* DxgiFormat_P010 */					24,
	/* DxgiFormat_P016 */					24,
	/* DxgiFormat_420Opaque */				12,
	/* DxgiFormat_Yuy2 */					32,
	/* DxgiFormat_Y210 */					64,
	/* DxgiFormat_Y216 */					64,
	/* DxgiFormat_Nv11 */					12,
	/* DxgiFormat_Ai44 */					8,
	/* DxgiFormat_Ia44 */					8,
	/* DxgiFormat_P8 */						8,
	/* DxgiFormat_A8P8 */					16,
	/* DxgiFormat_B4G4R4A4Unorm */			16,
	/* DxgiFormat_P208 */					0,	/* ??? */
	/* DxgiFormat_V208 */					0,	/* ??? */
	/* DxgiFormat_V408 */					0,	/* ??? */
	/* DxgiFormat_ForceUint */				0,	/* ??? */
};
static int GetNumBitsPerPixelsFromDxgiFormat(DxgiFormat dxgiFormat){
	if (dxgiFormat >= SIZE_OF_ARRAY(s_tblNumBitsPerPixel)) return 0;
	return s_tblNumBitsPerPixel[dxgiFormat];
}

static const DdsHeader *DdsParser_GetHeader(
	const DdsParser *parser
){
	if (parser == NULL) return NULL;
	if (parser->fileImage == NULL) return NULL;
	if (parser->fileSizeInBytes < sizeof(DdsHeader)) return NULL;
	const DdsHeader *header = (const DdsHeader *)parser->fileImage;
	if (header->dwMagic != 0x20534444) return NULL;
	return header;
}

static const DdsHeaderDx10 *DdsParser_GetHeaderDx10(
	const DdsParser *parser
){
	if (parser == NULL) return NULL;
	const DdsHeader *header = DdsParser_GetHeader(parser);
	if (header == NULL) return NULL;
	uint32_t pfFlags = header->ddspf.dwFlags;
	uint32_t fourCc = header->ddspf.dwFourCc;
	if ((pfFlags & Ddpf_FourCc) == 0 || fourCc != DdsdFourCc_Dx10) return NULL;
	if (parser->fileSizeInBytes < sizeof(DdsHeader) + sizeof(DdsHeaderDx10)) return NULL;
	const DdsHeaderDx10 *headerDx10 = (const DdsHeaderDx10 *)(header + 1);
	return headerDx10;
}

static DxgiFormat DdsParser_GetDxgiFormat(
	const DdsParser *parser
){
	if (parser == NULL) return DxgiFormat_Unknown;
	const DdsHeader *header = DdsParser_GetHeader(parser);
	if (header == NULL) return DxgiFormat_Unknown;
	const DdsHeaderDx10 *headerDx10 = DdsParser_GetHeaderDx10(parser);
	if (headerDx10) return (DxgiFormat)headerDx10->dxgiFormat;

	uint32_t pfFlags = header->ddspf.dwFlags;
	uint32_t fourCc = header->ddspf.dwFourCc;
	DxgiFormat dxgiFormat = DxgiFormat_Unknown;

	#undef	MACRO
	#define	MACRO(\
		header, rgbBitCount, rBitMask, gBitMask, bBitMask, aBitMask\
	)(\
		header->ddspf.dwRgbBitCount == rgbBitCount\
	&&	(\
			header->ddspf.dwRBitMask == rBitMask\
		&&	header->ddspf.dwGBitMask == gBitMask\
		&&	header->ddspf.dwBBitMask == bBitMask\
		&&	header->ddspf.dwABitMask == aBitMask\
		)\
	)\

	if (pfFlags & Ddpf_Rgb) {
		if (MACRO(header, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)) {
			dxgiFormat = DxgiFormat_R8G8B8A8Unorm;
		} else
		if (MACRO(header, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000)) {
			dxgiFormat = DxgiFormat_B8G8R8A8Unorm;
		} else
		if (MACRO(header, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000)) {
			dxgiFormat = DxgiFormat_B8G8R8X8Unorm;
		} else
		if (MACRO(header, 32, 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000)) {
			dxgiFormat = DxgiFormat_R10G10B10A2Unorm;
		} else
		if (MACRO(header, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000)) {
			dxgiFormat = DxgiFormat_R16G16Unorm;
		} else
		if (MACRO(header, 32, 0xffffffff, 0x00000000, 0x00000000, 0x00000000)) {
			dxgiFormat = DxgiFormat_R32Float;
		} else
		if (MACRO(header, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000)) {
			dxgiFormat = DxgiFormat_B5G5R5A1Unorm;
		} else
		if (MACRO(header, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000)) {
			dxgiFormat = DxgiFormat_B5G6R5Unorm;
		} else
		if (MACRO(header, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000)) {
			dxgiFormat = DxgiFormat_B4G4R4A4Unorm;
		}
	} else
	if (pfFlags & Ddpf_Luminance) {
		if (MACRO(header, 8, 0x000000ff, 0x00000000, 0x00000000, 0x00000000)) {
			dxgiFormat = DxgiFormat_R8Unorm;
		} else
		if (MACRO(header, 16, 0x0000ffff, 0x00000000, 0x00000000, 0x00000000)) {
			dxgiFormat = DxgiFormat_R16Unorm;
		} else
		if (MACRO(header, 16, 0x000000ff, 0x00000000, 0x00000000, 0x0000ff00)) {
			dxgiFormat = DxgiFormat_R8G8Unorm;
		}
	} else
	if (pfFlags & Ddpf_Alpha) {
		if (header->ddspf.dwRgbBitCount == 8) {
			dxgiFormat = DxgiFormat_A8Unorm;
		}
	} else
	if (pfFlags & Ddpf_BumpDuDy) {
		if (MACRO(header, 16, 0x000000ff, 0x0000ff00, 0x00000000, 0x00000000)) {
			dxgiFormat = DxgiFormat_R8G8Snorm;
		} else
		if (MACRO(header, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)) {
			dxgiFormat = DxgiFormat_R8G8B8A8Snorm;
		} else
		if (MACRO(header, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000)) {
			dxgiFormat = DxgiFormat_R16G16Snorm;
		}
	} else
	if (pfFlags & Ddpf_FourCc) {
		switch (fourCc) {
			case DdsdFourCc_Dxt1:			dxgiFormat = DxgiFormat_Bc1Unorm; break;
			case DdsdFourCc_Dxt2:
			case DdsdFourCc_Dxt3:			dxgiFormat = DxgiFormat_Bc2Unorm; break;
			case DdsdFourCc_Dxt4:
			case DdsdFourCc_Dxt5:			dxgiFormat = DxgiFormat_Bc3Unorm; break;
			case DdsdFourCc_3dcAti1:
			case DdsdFourCc_Bc4Unorm:		dxgiFormat = DxgiFormat_Bc4Unorm; break;
			case DdsdFourCc_Bc4Snorm:		dxgiFormat = DxgiFormat_Bc4Snorm; break;
			case DdsdFourCc_3dcAti2:
			case DdsdFourCc_Bc5Unorm:		dxgiFormat = DxgiFormat_Bc5Unorm; break;
			case DdsdFourCc_Bc5Snorm:		dxgiFormat = DxgiFormat_Bc5Snorm; break;
			case DdsdFourCc_Rgbg:			dxgiFormat = DxgiFormat_R8G8B8G8Unorm; break;
			case DdsdFourCc_Grgb:			dxgiFormat = DxgiFormat_G8R8G8B8Unorm; break;
			case DdsdFourCc_Yuy2:			dxgiFormat = DxgiFormat_Yuy2; break;
			case DdsdFourCc_A16B16G16R16:	dxgiFormat = DxgiFormat_R16G16B16A16Unorm; break;
			case DdsdFourCc_Q16W16V16U16:	dxgiFormat = DxgiFormat_R16G16B16A16Snorm; break;
			case DdsdFourCc_R16F:			dxgiFormat = DxgiFormat_R16Float; break;
			case DdsdFourCc_G16R16F:		dxgiFormat = DxgiFormat_R16G16Float; break;
			case DdsdFourCc_A16B16G16R16F:	dxgiFormat = DxgiFormat_R16G16B16A16Float; break;
			case DdsdFourCc_R32F:			dxgiFormat = DxgiFormat_R32Float; break;
			case DdsdFourCc_G32R32F:		dxgiFormat = DxgiFormat_R32G32Float; break;
			case DdsdFourCc_A32B32G32R32F:	dxgiFormat = DxgiFormat_R32G32B32A32Float; break;
		}
	}
	return dxgiFormat;
}

bool DdsParser_Initialize(
	DdsParser *parser,
	const void *fileImage,
	size_t fileSizeInBytes
){
	if (parser == NULL) return false;

	parser->fileImage = fileImage;
	parser->fileSizeInBytes = fileSizeInBytes;

	const DdsHeader *header = DdsParser_GetHeader(parser);
	if (header == NULL) return false;
	const DdsHeaderDx10 *headerDx10 = DdsParser_GetHeaderDx10(parser);

	parser->info.dxgiFormat = DdsParser_GetDxgiFormat(parser);
	parser->info.width  = header->dwWidth;
	parser->info.height = header->dwHeight;
	parser->info.depth  = IntMax(header->dwDepth, 1);
	parser->info.arraySize = (headerDx10 != NULL)? IntMax(headerDx10->arraySize, 1): 1;
	parser->info.hasCubemap = (header->dwCaps2 & DdsCaps2_Cubemap) != 0;
	parser->info.numMips = IntMax(header->dwMipMapCount, 1);
	parser->info.blockCompressed = (
		parser->info.dxgiFormat == DxgiFormat_Bc1Typeless
	||	parser->info.dxgiFormat == DxgiFormat_Bc1Unorm
	||	parser->info.dxgiFormat == DxgiFormat_Bc1UnormSrgb

	||	parser->info.dxgiFormat == DxgiFormat_Bc2Typeless
	||	parser->info.dxgiFormat == DxgiFormat_Bc2Unorm
	||	parser->info.dxgiFormat == DxgiFormat_Bc2UnormSrgb

	||	parser->info.dxgiFormat == DxgiFormat_Bc3Typeless
	||	parser->info.dxgiFormat == DxgiFormat_Bc3Unorm
	||	parser->info.dxgiFormat == DxgiFormat_Bc3UnormSrgb

	||	parser->info.dxgiFormat == DxgiFormat_Bc4Typeless
	||	parser->info.dxgiFormat == DxgiFormat_Bc4Unorm
	||	parser->info.dxgiFormat == DxgiFormat_Bc4Snorm

	||	parser->info.dxgiFormat == DxgiFormat_Bc5Typeless
	||	parser->info.dxgiFormat == DxgiFormat_Bc5Unorm
	||	parser->info.dxgiFormat == DxgiFormat_Bc5Snorm

	||	parser->info.dxgiFormat == DxgiFormat_Bc6hTypeless
	||	parser->info.dxgiFormat == DxgiFormat_Bc6hUf16
	||	parser->info.dxgiFormat == DxgiFormat_Bc6hSf16

	||	parser->info.dxgiFormat == DxgiFormat_Bc7Typeless
	||	parser->info.dxgiFormat == DxgiFormat_Bc7Unorm
	||	parser->info.dxgiFormat == DxgiFormat_Bc7UnormSrgb
	)? 1: 0;
	parser->info.numBitsPerPixel = GetNumBitsPerPixelsFromDxgiFormat(parser->info.dxgiFormat);

	return true;
}

bool DdsParser_GetSubData(
	const DdsParser *parser,
	int arrayIndex,
	int faceIndex,
	int mipIndex,
	DdsSubData *subData
){
	if (parser == NULL) return false;
	const DdsHeader *header = DdsParser_GetHeader(parser);
	if (header == NULL) return false;
	const DdsHeaderDx10 *headerDx10 = DdsParser_GetHeaderDx10(parser);

	size_t offset = sizeof(DdsHeader);
	if (headerDx10 != NULL) offset += sizeof(DdsHeaderDx10);

	int numFaces = (parser->info.hasCubemap)? 6: 1;

	for (int arraySearchIndex = 0; arraySearchIndex < parser->info.arraySize; arraySearchIndex++) {
		for (int faceSearchIndex = 0; faceSearchIndex < numFaces; faceSearchIndex++) {
			for (int mipSearchIndex = 0; mipSearchIndex < parser->info.numMips; mipSearchIndex++) {
				int	mipWidth  = parser->info.width  >> mipSearchIndex;
				int	mipHeight = parser->info.height >> mipSearchIndex;
				int	mipDepth  = parser->info.depth  >> mipSearchIndex;
				if (mipWidth  <= 0) mipWidth  = 1;
				if (mipHeight <= 0) mipHeight = 1;
				if (mipDepth  <= 0) mipDepth  = 1;

				size_t rowPitchInBytes =
					((parser->info.blockCompressed)? IntCeilAlign(mipWidth, 4): mipWidth)
				*	parser->info.numBitsPerPixel / 8;
				size_t slicePitchInBytes =
					((parser->info.blockCompressed)? IntCeilAlign(mipHeight, 4): mipHeight)
				*	rowPitchInBytes;
				size_t sizeInBytes = slicePitchInBytes * mipDepth;

				if (offset + sizeInBytes > parser->fileSizeInBytes) return false;

				if (arraySearchIndex == arrayIndex
				&&	faceSearchIndex == faceIndex
				&&	mipSearchIndex == mipIndex
				) {
					subData->buff = (const void *)((uintptr_t)parser->fileImage + offset);
					subData->width  = mipWidth;
					subData->height = mipHeight;
					subData->depth  = mipDepth;
					subData->sizeInBytes = sizeInBytes;
					subData->rowPitchInBytes = rowPitchInBytes;
					subData->slicePitchInBytes = slicePitchInBytes;
					return true;
				}

				offset += sizeInBytes;
			}
		}
	}

	return false;
}

