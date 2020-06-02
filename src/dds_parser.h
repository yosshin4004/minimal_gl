/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DDS_PARSER_H_
#define _DDS_PARSER_H_


struct DdsHeaderDx10 {
	uint32_t		dxgiFormat;
	uint32_t		resourceDimension;
	uint32_t		miscFlag;
	uint32_t		arraySize;
	uint32_t		miscFlags2;
};

struct DdsPixelFormat {
	uint32_t		dwSize;
	uint32_t		dwFlags;
	uint32_t		dwFourCC;
	uint32_t		dwRGBBitCount;
	uint32_t		dwRBitMask;
	uint32_t		dwGBitMask;
	uint32_t		dwBBitMask;
	uint32_t		dwABitMask;
};

struct DdsHeader {
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
};

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

	DdsdFourCC_Rgbg				= 'R' | ((int)'G' << 8) | ((int)'B' << 16) | ((int)'G' << 24),
	DdsdFourCC_Grgb				= 'G' | ((int)'R' << 8) | ((int)'G' << 16) | ((int)'B' << 24),
	DdsdFourCC_Yuy2				= 'Y' | ((int)'U' << 8) | ((int)'Y' << 16) | ((int)'2' << 24),

	DdsdFourCC_A16B16G16R16		= 0x00000024,
	DdsdFourCC_Q16W16V16U16		= 0x0000006e,
	DdsdFourCC_R16F				= 0x0000006f,
	DdsdFourCC_G16R16F			= 0x00000070,
	DdsdFourCC_A16B16G16R16F	= 0x00000071,
	DdsdFourCC_R32F				= 0x00000072,
	DdsdFourCC_G32R32F			= 0x00000073,
	DdsdFourCC_A32B32G32R32F	= 0x00000074,

	DdsdFourCC_3dcAti1			= 'A' | ((int)'T' << 8) | ((int)'I' << 16) | ((int)'1' << 24),
	DdsdFourCC_3dcAti2			= 'A' | ((int)'T' << 8) | ((int)'I' << 16) | ((int)'2' << 24),
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

typedef enum {
	DxgiFormat_Unknown					= 0,
	DxgiFormat_R32G32B32A32Typeless		= 1,
	DxgiFormat_R32G32B32A32Float		= 2,
	DxgiFormat_R32G32B32A32Uint			= 3,
	DxgiFormat_R32G32B32A32Sint			= 4,

	DxgiFormat_R32G32B32Typeless		= 5,
	DxgiFormat_R32G32B32Float			= 6,
	DxgiFormat_R32G32B32Uint			= 7,
	DxgiFormat_R32G32B32Sint			= 8,

	DxgiFormat_R16G16B16A16Typeless		= 9,
	DxgiFormat_R16G16B16A16Float		= 10,
	DxgiFormat_R16G16B16A16Unorm		= 11,
	DxgiFormat_R16G16B16A16Uint			= 12,
	DxgiFormat_R16G16B16A16Snorm		= 13,
	DxgiFormat_R16G16B16A16Sint			= 14,

	DxgiFormat_R32G32Typeless			= 15,
	DxgiFormat_R32G32Float				= 16,
	DxgiFormat_R32G32Uint				= 17,
	DxgiFormat_R32G32Sint				= 18,

	DxgiFormat_R32G8X24Typeless			= 19,
	DxgiFormat_D32FloatS8X24Uint		= 20,
	DxgiFormat_R32FloatX8X24Typeless	= 21,
	DxgiFormat_X32TypelessG8X24Uint		= 22,

	DxgiFormat_R10G10B10A2Typeless		= 23,
	DxgiFormat_R10G10B10A2Unorm			= 24,
	DxgiFormat_R10G10B10A2Uint			= 25,

	DxgiFormat_R11G11B10Float			= 26,

	DxgiFormat_R8G8B8A8Typeless			= 27,
	DxgiFormat_R8G8B8A8Unorm			= 28,
	DxgiFormat_R8G8B8A8UnormSrgb		= 29,
	DxgiFormat_R8G8B8A8Uint				= 30,
	DxgiFormat_R8G8B8A8Snorm			= 31,
	DxgiFormat_R8G8B8A8Sint				= 32,

	DxgiFormat_R16G16Typeless			= 33,
	DxgiFormat_R16G16Float				= 34,
	DxgiFormat_R16G16Unorm				= 35,
	DxgiFormat_R16G16Uint				= 36,
	DxgiFormat_R16G16Snorm				= 37,
	DxgiFormat_R16G16Sint				= 38,

	DxgiFormat_R32Typeless				= 39,
	DxgiFormat_D32Float					= 40,
	DxgiFormat_R32Float					= 41,
	DxgiFormat_R32Uint					= 42,
	DxgiFormat_R32Sint					= 43,

	DxgiFormat_R24G8Typeless			= 44,
	DxgiFormat_D24UnormS8Uint			= 45,
	DxgiFormat_R24UnormX8Typeless		= 46,
	DxgiFormat_X24TypelessG8Uint		= 47,

	DxgiFormat_R8G8Typeless				= 48,
	DxgiFormat_R8G8Unorm				= 49,
	DxgiFormat_R8G8Uint					= 50,
	DxgiFormat_R8G8Snorm				= 51,
	DxgiFormat_R8G8Sint					= 52,

	DxgiFormat_R16Typeless				= 53,
	DxgiFormat_R16Float					= 54,
	DxgiFormat_D16Unorm					= 55,
	DxgiFormat_R16Unorm					= 56,
	DxgiFormat_R16Uint					= 57,
	DxgiFormat_R16Snorm					= 58,
	DxgiFormat_R16Sint					= 59,

	DxgiFormat_R8Typeless				= 60,
	DxgiFormat_R8Unorm					= 61,
	DxgiFormat_R8Uint					= 62,
	DxgiFormat_R8Snorm					= 63,
	DxgiFormat_R8Sint					= 64,
	DxgiFormat_A8Unorm					= 65,
	DxgiFormat_R1Unorm					= 66,

	DxgiFormat_R9G9B9E5SharedExp		= 67,

	DxgiFormat_R8G8B8G8Unorm			= 68,
	DxgiFormat_G8R8G8B8Unorm			= 69,

	DxgiFormat_Bc1Typeless				= 70,
	DxgiFormat_Bc1Unorm					= 71,
	DxgiFormat_Bc1UnormSrgb				= 72,

	DxgiFormat_Bc2Typeless				= 73,
	DxgiFormat_Bc2Unorm					= 74,
	DxgiFormat_Bc2UnormSrgb				= 75,

	DxgiFormat_Bc3Typeless				= 76,
	DxgiFormat_Bc3Unorm					= 77,
	DxgiFormat_Bc3UnormSrgb				= 78,

	DxgiFormat_Bc4Typeless				= 79,
	DxgiFormat_Bc4Unorm					= 80,
	DxgiFormat_Bc4Snorm					= 81,

	DxgiFormat_Bc5Typeless				= 82,
	DxgiFormat_Bc5Unorm					= 83,
	DxgiFormat_Bc5Snorm					= 84,

	DxgiFormat_B5G6R5Unorm				= 85,
	DxgiFormat_B5G5R5A1Unorm			= 86,
	DxgiFormat_B8G8R8A8Unorm			= 87,
	DxgiFormat_B8G8R8X8Unorm			= 88,
	DxgiFormat_R10G10B10XrBiasA2Unorm	= 89,
	DxgiFormat_B8G8R8A8Typeless			= 90,
	DxgiFormat_B8G8R8A8UnormSrgb		= 91,
	DxgiFormat_B8G8R8X8Typeless			= 92,
	DxgiFormat_B8G8R8X8UnormSrgb		= 93,

	DxgiFormat_Bc6hTypeless				= 94,
	DxgiFormat_Bc6hUf16					= 95,
	DxgiFormat_Bc6hSf16					= 96,

	DxgiFormat_Bc7Typeless				= 97,
	DxgiFormat_Bc7Unorm					= 98,
	DxgiFormat_Bc7UnormSrgb				= 99,

	DxgiFormat_Ayuv						= 100,
	DxgiFormat_Y410						= 101,
	DxgiFormat_Y416						= 102,
	DxgiFormat_Nv12						= 103,
	DxgiFormat_P010						= 104,
	DxgiFormat_P016						= 105,
	DxgiFormat_420Opaque				= 106,
	DxgiFormat_Yuy2						= 107,
	DxgiFormat_Y210						= 108,
	DxgiFormat_Y216						= 109,
	DxgiFormat_Nv11						= 110,
	DxgiFormat_Ai44						= 111,
	DxgiFormat_Ia44						= 112,
	DxgiFormat_P8						= 113,
	DxgiFormat_A8P8						= 114,
	DxgiFormat_B4G4R4A4Unorm			= 115,
} DxgiFormat;

struct DdsInfo {
	DxgiFormat dxgiFormat;
	int numBitsPerPixel;
	int width;
	int height;
	int depth;
	int arraySize;
	int hasCubemap;
	int numMips;
	int blockCompressed;
};
struct DdsParser {
	const void *fileImage;
	size_t fileSizeInBytes;
	DdsInfo info;
};
bool DdsParser_Initialize(
	DdsParser *parser,
	const void *fileImage,
	size_t fileSizeInBytes
);

struct DdsSubData {
	const void *buff;
	int width;
	int height;
	int depth;
	size_t sizeInBytes;
	size_t rowPitchInBytes;
	size_t slicePitchInBytes;
};
bool DdsParser_GetSubData(
	const DdsParser *parser,
	int arrayIndex,
	int faceIndex,
	int mipIndex,
	DdsSubData *subData
);


#endif
