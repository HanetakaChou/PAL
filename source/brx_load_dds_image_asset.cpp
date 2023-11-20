//
// Copyright (C) YuqiaoZhang(HanetakaChou)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#include <stddef.h>
#include <assert.h>
#include <algorithm>
#include "brx_load_dds_image_asset.h"

extern bool brx_load_dds_image_asset_header_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER *image_asset_header, size_t *image_asset_data_offset);

extern bool brx_load_dds_image_asset_data_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER const *image_asset_header, size_t image_asset_data_offset, void *staging_upload_buffer_base, size_t subresource_count, BRX_LOAD_IMAGE_ASSET_SUBRESOURCE_MEMCPY_DEST const *subresource_memcpy_dests);

//--------------------------------------------------------------------------------------
// DDS file structure definitions
//
// https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds
//--------------------------------------------------------------------------------------

static inline constexpr uint32_t MakeFourCC(char ch0, char ch1, char ch2, char ch3)
{
    return static_cast<uint32_t>(static_cast<uint8_t>(ch0)) | (static_cast<uint32_t>(static_cast<uint8_t>(ch1)) << 8U) | (static_cast<uint32_t>(static_cast<uint8_t>(ch2)) << 16U) | (static_cast<uint32_t>(static_cast<uint8_t>(ch3)) << 24U);
}

enum : uint32_t
{
    DDS_MAGIC = MakeFourCC('D', 'D', 'S', ' ')
};

enum : uint32_t
{
    DDPF_ALPHA = 0x00000002,
    DDPF_FOURCC = 0x00000004,
    DDPF_RGB = 0x00000040,
    DDPF_LUMINANCE = 0x00020000,
    DDPF_BUMPDUDV = 0x00080000
};

struct DDS_PIXELFORMAT
{
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwFourCC;
    uint32_t dwRGBBitCount;
    uint32_t dwRBitMask;
    uint32_t dwGBitMask;
    uint32_t dwBBitMask;
    uint32_t dwABitMask;
};

enum : uint32_t
{
    DDSD_HEIGHT = 0x00000002,
    DDSD_DEPTH = 0x00800000
};

enum : uint32_t
{
    DDSCAPS2_CUBEMAP = 0x00000200,
    DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400,
    DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800,
    DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000,
    DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000,
    DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000,
    DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000
};

struct DDS_HEADER
{
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwHeight;
    uint32_t dwWidth;
    uint32_t dwPitchOrLinearSize;
    uint32_t dwDepth; // only if DDSD_DEPTH is set in flags
    uint32_t dwMipMapCount;
    uint32_t dwReserved1[11];
    struct DDS_PIXELFORMAT ddspf;
    uint32_t dwCaps;
    uint32_t dwCaps2;
    uint32_t dwCaps3;
    uint32_t dwCaps4;
    uint32_t dwReserved2;
};

enum DDS_FORMAT : uint32_t
{
    DDS_FORMAT_UNKNOWN = 0,
    DDS_FORMAT_R32G32B32A32_TYPELESS = 1,
    DDS_FORMAT_R32G32B32A32_FLOAT = 2,
    DDS_FORMAT_R32G32B32A32_UINT = 3,
    DDS_FORMAT_R32G32B32A32_SINT = 4,
    DDS_FORMAT_R32G32B32_TYPELESS = 5,
    DDS_FORMAT_R32G32B32_FLOAT = 6,
    DDS_FORMAT_R32G32B32_UINT = 7,
    DDS_FORMAT_R32G32B32_SINT = 8,
    DDS_FORMAT_R16G16B16A16_TYPELESS = 9,
    DDS_FORMAT_R16G16B16A16_FLOAT = 10,
    DDS_FORMAT_R16G16B16A16_UNORM = 11,
    DDS_FORMAT_R16G16B16A16_UINT = 12,
    DDS_FORMAT_R16G16B16A16_SNORM = 13,
    DDS_FORMAT_R16G16B16A16_SINT = 14,
    DDS_FORMAT_R32G32_TYPELESS = 15,
    DDS_FORMAT_R32G32_FLOAT = 16,
    DDS_FORMAT_R32G32_UINT = 17,
    DDS_FORMAT_R32G32_SINT = 18,
    DDS_FORMAT_R32G8X24_TYPELESS = 19,
    DDS_FORMAT_D32_FLOAT_S8X24_UINT = 20,
    DDS_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
    DDS_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
    DDS_FORMAT_R10G10B10A2_TYPELESS = 23,
    DDS_FORMAT_R10G10B10A2_UNORM = 24,
    DDS_FORMAT_R10G10B10A2_UINT = 25,
    DDS_FORMAT_R11G11B10_FLOAT = 26,
    DDS_FORMAT_R8G8B8A8_TYPELESS = 27,
    DDS_FORMAT_R8G8B8A8_UNORM = 28,
    DDS_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
    DDS_FORMAT_R8G8B8A8_UINT = 30,
    DDS_FORMAT_R8G8B8A8_SNORM = 31,
    DDS_FORMAT_R8G8B8A8_SINT = 32,
    DDS_FORMAT_R16G16_TYPELESS = 33,
    DDS_FORMAT_R16G16_FLOAT = 34,
    DDS_FORMAT_R16G16_UNORM = 35,
    DDS_FORMAT_R16G16_UINT = 36,
    DDS_FORMAT_R16G16_SNORM = 37,
    DDS_FORMAT_R16G16_SINT = 38,
    DDS_FORMAT_R32_TYPELESS = 39,
    DDS_FORMAT_D32_FLOAT = 40,
    DDS_FORMAT_R32_FLOAT = 41,
    DDS_FORMAT_R32_UINT = 42,
    DDS_FORMAT_R32_SINT = 43,
    DDS_FORMAT_R24G8_TYPELESS = 44,
    DDS_FORMAT_D24_UNORM_S8_UINT = 45,
    DDS_FORMAT_R24_UNORM_X8_TYPELESS = 46,
    DDS_FORMAT_X24_TYPELESS_G8_UINT = 47,
    DDS_FORMAT_R8G8_TYPELESS = 48,
    DDS_FORMAT_R8G8_UNORM = 49,
    DDS_FORMAT_R8G8_UINT = 50,
    DDS_FORMAT_R8G8_SNORM = 51,
    DDS_FORMAT_R8G8_SINT = 52,
    DDS_FORMAT_R16_TYPELESS = 53,
    DDS_FORMAT_R16_FLOAT = 54,
    DDS_FORMAT_D16_UNORM = 55,
    DDS_FORMAT_R16_UNORM = 56,
    DDS_FORMAT_R16_UINT = 57,
    DDS_FORMAT_R16_SNORM = 58,
    DDS_FORMAT_R16_SINT = 59,
    DDS_FORMAT_R8_TYPELESS = 60,
    DDS_FORMAT_R8_UNORM = 61,
    DDS_FORMAT_R8_UINT = 62,
    DDS_FORMAT_R8_SNORM = 63,
    DDS_FORMAT_R8_SINT = 64,
    DDS_FORMAT_A8_UNORM = 65,
    DDS_FORMAT_R1_UNORM = 66,
    DDS_FORMAT_R9G9B9E5_SHAREDEXP = 67,
    DDS_FORMAT_R8G8_B8G8_UNORM = 68,
    DDS_FORMAT_G8R8_G8B8_UNORM = 69,
    DDS_FORMAT_BC1_TYPELESS = 70,
    DDS_FORMAT_BC1_UNORM = 71,
    DDS_FORMAT_BC1_UNORM_SRGB = 72,
    DDS_FORMAT_BC2_TYPELESS = 73,
    DDS_FORMAT_BC2_UNORM = 74,
    DDS_FORMAT_BC2_UNORM_SRGB = 75,
    DDS_FORMAT_BC3_TYPELESS = 76,
    DDS_FORMAT_BC3_UNORM = 77,
    DDS_FORMAT_BC3_UNORM_SRGB = 78,
    DDS_FORMAT_BC4_TYPELESS = 79,
    DDS_FORMAT_BC4_UNORM = 80,
    DDS_FORMAT_BC4_SNORM = 81,
    DDS_FORMAT_BC5_TYPELESS = 82,
    DDS_FORMAT_BC5_UNORM = 83,
    DDS_FORMAT_BC5_SNORM = 84,
    DDS_FORMAT_B5G6R5_UNORM = 85,
    DDS_FORMAT_B5G5R5A1_UNORM = 86,
    DDS_FORMAT_B8G8R8A8_UNORM = 87,
    DDS_FORMAT_B8G8R8X8_UNORM = 88,
    DDS_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
    DDS_FORMAT_B8G8R8A8_TYPELESS = 90,
    DDS_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
    DDS_FORMAT_B8G8R8X8_TYPELESS = 92,
    DDS_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
    DDS_FORMAT_BC6H_TYPELESS = 94,
    DDS_FORMAT_BC6H_UF16 = 95,
    DDS_FORMAT_BC6H_SF16 = 96,
    DDS_FORMAT_BC7_TYPELESS = 97,
    DDS_FORMAT_BC7_UNORM = 98,
    DDS_FORMAT_BC7_UNORM_SRGB = 99,
    DDS_FORMAT_AYUV = 100,
    DDS_FORMAT_Y410 = 101,
    DDS_FORMAT_Y416 = 102,
    DDS_FORMAT_NV12 = 103,
    DDS_FORMAT_P010 = 104,
    DDS_FORMAT_P016 = 105,
    DDS_FORMAT_420_OPAQUE = 106,
    DDS_FORMAT_YUY2 = 107,
    DDS_FORMAT_Y210 = 108,
    DDS_FORMAT_Y216 = 109,
    DDS_FORMAT_NV11 = 110,
    DDS_FORMAT_AI44 = 111,
    DDS_FORMAT_IA44 = 112,
    DDS_FORMAT_P8 = 113,
    DDS_FORMAT_A8P8 = 114,
    DDS_FORMAT_B4G4R4A4_UNORM = 115,
    DDS_FORMAT_P208 = 130,
    DDS_FORMAT_V208 = 131,
    DDS_FORMAT_V408 = 132
};

enum
{
    DDS_RESOURCE_DIMENSION_UNKNOWN = 0,
    DDS_RESOURCE_DIMENSION_UNKNOWN_BUFFER = 1,
    DDS_RESOURCE_DIMENSION_TEXTURE1D = 2,
    DDS_RESOURCE_DIMENSION_TEXTURE2D = 3,
    DDS_RESOURCE_DIMENSION_TEXTURE3D = 4
};

enum
{
    DDS_RESOURCE_MISC_TEXTURECUBE = 0x4L
};

enum
{
    DDS_ALPHA_MODE_UNKNOWN = 0,
    DDS_ALPHA_MODE_STRAIGHT = 1,
    DDS_ALPHA_MODE_PREMULTIPLIED = 2,
    DDS_ALPHA_MODE_OPAQUE = 3,
    DDS_ALPHA_MODE_CUSTOM = 4
};

enum
{
    DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L
};

struct DDS_HEADER_DXT10
{
    DDS_FORMAT ddsFormat;
    uint32_t resourceDimension;
    uint32_t miscFlag;
    uint32_t arraySize;
    uint32_t miscFlags2;
};

//--------------------------------------------------------------------------------------
static inline DDS_FORMAT GetDDSFormat(struct DDS_PIXELFORMAT const *ddpf);

static inline size_t BitsPerPixel(uint32_t fmt);

static inline bool GetSurfaceInfo(size_t width, size_t height, uint32_t fmt, size_t *outNumBytes, size_t *outRowBytes, size_t *outNumRows);

static inline uint32_t DDSGetFormatPlaneCount(DDS_FORMAT dds_format);

static inline bool IsDepthStencil(DDS_FORMAT dds_format);

//--------------------------------------------------------------------------------------
extern bool brx_load_dds_image_asset_header_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER *image_asset_header, size_t *image_asset_data_offset)
{
    assert(image_asset_header != NULL);
    assert(image_asset_data_offset != NULL);

    if (-1 == input_stream->seek(0, LOAD_ASSET_INPUT_STREAM_SEEK_SET))
    {
        return false;
    }

    uint8_t ddsDataBuf[sizeof(uint32_t) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10)];
    uint8_t const *const ddsData = ddsDataBuf;
    {
        ptrdiff_t const BytesRead = input_stream->read(ddsDataBuf, sizeof(uint32_t) + sizeof(DDS_HEADER));
        if (BytesRead == -1 || static_cast<size_t>(BytesRead) < (sizeof(uint32_t) + sizeof(DDS_HEADER)))
        {
            return false;
        }
    }

    // DDS files always start with the same magic number ("DDS ")
    uint32_t const *const dwMagicNumber = reinterpret_cast<const uint32_t *>(ddsData);
    if (DDS_MAGIC != (*dwMagicNumber))
    {
        return false;
    }

    DDS_HEADER const *const header = reinterpret_cast<struct DDS_HEADER const *>(ddsData + sizeof(uint32_t));
    // Verify header to validate DDS file
    if (header->dwSize != sizeof(DDS_HEADER) || header->ddspf.dwSize != sizeof(DDS_PIXELFORMAT))
    {
        return false;
    }

    // Check for DX10 extension
    DDS_HEADER_DXT10 const *d3d10ext = NULL;
    if ((header->ddspf.dwFlags & DDPF_FOURCC) && (MakeFourCC('D', 'X', '1', '0') == header->ddspf.dwFourCC))
    {
        // Must be long enough for both headers and magic value
        ptrdiff_t BytesRead = input_stream->read(ddsDataBuf + (sizeof(uint32_t) + sizeof(DDS_HEADER)), sizeof(DDS_HEADER_DXT10));
        if (BytesRead == -1 || static_cast<size_t>(BytesRead) < sizeof(DDS_HEADER_DXT10))
        {
            return false;
        }

        d3d10ext = reinterpret_cast<struct DDS_HEADER_DXT10 const *>(ddsData + (sizeof(uint32_t) + sizeof(DDS_HEADER)));
    }

    (*image_asset_data_offset) = (d3d10ext != NULL) ? (sizeof(uint32_t) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10)) : (sizeof(uint32_t) + sizeof(DDS_HEADER));

    uint32_t const width = header->dwWidth;
    uint32_t height = header->dwHeight;
    uint32_t depth = header->dwDepth;

    BRX_ASSET_IMAGE_TYPE res_dim = static_cast<BRX_ASSET_IMAGE_TYPE>(-1);
    uint32_t array_size = 1U;
    DDS_FORMAT dds_format = DDS_FORMAT_UNKNOWN;
    bool is_cube_map = false;

    uint32_t mip_count = header->dwMipMapCount;
    if (0U == mip_count)
    {
        mip_count = 1;
    }

    if (NULL != d3d10ext)
    {
        array_size = d3d10ext->arraySize;
        if (0U == array_size)
        {
            return false;
        }

        switch (d3d10ext->ddsFormat)
        {
        case DDS_FORMAT_NV12:
        case DDS_FORMAT_P010:
        case DDS_FORMAT_P016:
        case DDS_FORMAT_420_OPAQUE:
            if ((DDS_RESOURCE_DIMENSION_TEXTURE2D != d3d10ext->resourceDimension) || 0U != (width % 2U) || 0U != (height % 2U))
            {
                return false;
            }
            break;

        case DDS_FORMAT_YUY2:
        case DDS_FORMAT_Y210:
        case DDS_FORMAT_Y216:
        case DDS_FORMAT_P208:
            if (0U != (width % 2U))
            {
                return false;
            }
            break;

        case DDS_FORMAT_NV11:
            if (0U != (width % 4U))
            {
                return false;
            }
            break;

        case DDS_FORMAT_AI44:
        case DDS_FORMAT_IA44:
        case DDS_FORMAT_P8:
        case DDS_FORMAT_A8P8:
            return false;

        case DDS_FORMAT_V208:
            if ((DDS_RESOURCE_DIMENSION_TEXTURE2D != d3d10ext->resourceDimension) || 0U != (height % 2U))
            {
                return false;
            }
            break;

        default:
            if (0U == BitsPerPixel(d3d10ext->ddsFormat))
            {
                return false;
            }
        }

        dds_format = d3d10ext->ddsFormat;

        switch (d3d10ext->resourceDimension)
        {
        case DDS_RESOURCE_DIMENSION_TEXTURE1D:
        {
            // D3DX writes 1D textures with a fixed Height of 1
            if ((header->dwFlags & DDSD_HEIGHT) && (1U != header->dwHeight))
            {
                return false;
            }
            height = 1U;
            depth = 1U;

            res_dim = BRX_ASSET_IMAGE_TYPE_1D;
        }
        break;
        case DDS_RESOURCE_DIMENSION_TEXTURE2D:
        {
            if (d3d10ext->miscFlag & DDS_RESOURCE_MISC_TEXTURECUBE)
            {
                array_size *= 6U;
                is_cube_map = true;
            }
            depth = 1;

            res_dim = BRX_ASSET_IMAGE_TYPE_2D;
        }
        break;
        case DDS_RESOURCE_DIMENSION_TEXTURE3D:
        {
            if (!(header->dwFlags & DDSD_DEPTH))
            {
                return false;
            }

            if (array_size > 1)
            {
                return false;
            }

            res_dim = BRX_ASSET_IMAGE_TYPE_3D;
        }
        break;
        default:
            return false;
        }
    }
    else
    {
        dds_format = GetDDSFormat(&header->ddspf);

        if (DDS_FORMAT_UNKNOWN == dds_format)
        {
            return false;
        }

        if (header->dwFlags & DDSD_DEPTH)
        {
            res_dim = BRX_ASSET_IMAGE_TYPE_3D;
        }
        else
        {
            if (header->dwCaps2 & DDSCAPS2_CUBEMAP)
            {
                // We require all six faces to be defined
                if ((header->dwCaps2 & (DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX | DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_NEGATIVEY | DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ)) != (DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX | DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_NEGATIVEY | DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ))
                {
                    return false;
                }

                array_size = 6U;
                is_cube_map = true;
            }

            depth = 1U;
            res_dim = BRX_ASSET_IMAGE_TYPE_2D;

            // Note there's no way for a legacy Direct3D 9 DDS to express a '1D' texture
        }

        assert(0U != BitsPerPixel(dds_format));
    }

    image_asset_header->is_cube_map = is_cube_map;
    image_asset_header->type = res_dim;
    switch (dds_format)
    {
    case DDS_FORMAT_R8G8B8A8_UNORM:
        image_asset_header->format = BRX_ASSET_IMAGE_FORMAT_R8G8B8A8_UNORM;
        break;
    case DDS_FORMAT_BC7_UNORM:
        image_asset_header->format = BRX_ASSET_IMAGE_FORMAT_BC7_UNORM_BLOCK;
        break;
    default:
        // TODO: support more format
        assert(false);
        image_asset_header->format = static_cast<BRX_ASSET_IMAGE_FORMAT>(-1);
        return false;
    }
    image_asset_header->width = width;
    image_asset_header->height = height;
    image_asset_header->depth = depth;
    image_asset_header->mip_levels = mip_count;
    image_asset_header->array_layers = array_size;
    return true;
}

extern bool brx_load_dds_image_asset_data_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER const *image_asset_header, size_t image_asset_data_offset, void *staging_upload_buffer_base, size_t subresource_count, BRX_LOAD_IMAGE_ASSET_SUBRESOURCE_MEMCPY_DEST const *subresource_memcpy_dests)
{
#ifndef NDEBUG
    BRX_LOAD_IMAGE_ASSET_HEADER image_asset_header_for_validate;
    size_t image_asset_data_offset_for_validate;
    if (!brx_load_dds_image_asset_header_from_input_stream(input_stream, &image_asset_header_for_validate, &image_asset_data_offset_for_validate))
    {
        return false;
    }

    assert(image_asset_header->is_cube_map == image_asset_header_for_validate.is_cube_map);
    assert(image_asset_header->type == image_asset_header_for_validate.type);
    assert(image_asset_header->format == image_asset_header_for_validate.format);
    assert(image_asset_header->width == image_asset_header_for_validate.width);
    assert(image_asset_header->height == image_asset_header_for_validate.height);
    assert(image_asset_header->depth == image_asset_header_for_validate.depth);
    assert(image_asset_header->mip_levels == image_asset_header_for_validate.mip_levels);
    assert(image_asset_header->array_layers == image_asset_header_for_validate.array_layers);

    assert(image_asset_data_offset == image_asset_data_offset_for_validate);
#endif

    if (-1 == input_stream->seek(image_asset_data_offset, LOAD_ASSET_INPUT_STREAM_SEEK_SET))
    {
        return false;
    }

    DDS_FORMAT dds_format;
    switch (image_asset_header->format)
    {
    case BRX_ASSET_IMAGE_FORMAT_R8G8B8A8_UNORM:
        dds_format = DDS_FORMAT_R8G8B8A8_UNORM;
        break;
    case BRX_ASSET_IMAGE_FORMAT_BC7_UNORM_BLOCK:
        dds_format = DDS_FORMAT_BC7_UNORM;
        break;
    default:
        assert(false);
        dds_format = static_cast<DDS_FORMAT>(-1);
    }

    uint32_t const numberOfPlanes = DDSGetFormatPlaneCount(dds_format);
    if (0U == numberOfPlanes)
    {
        return false;
    }

    if ((numberOfPlanes > 1U) && IsDepthStencil(dds_format))
    {
        // DirectX 12 uses planes for stencil, DirectX 11 does not
        return false;
    }

    size_t const numberOfResources = ((BRX_ASSET_IMAGE_TYPE_3D == image_asset_header->type) ? static_cast<size_t>(1U) : static_cast<size_t>(image_asset_header->array_layers)) * static_cast<size_t>(image_asset_header->mip_levels) * static_cast<size_t>(numberOfPlanes);

    if (numberOfResources != subresource_count)
    {
        return false;
    }

    size_t inputSkipBytes = image_asset_data_offset;

    // TODO: support more than one plane
    assert(1U == numberOfPlanes);
    for (uint32_t planeSlice = 0; planeSlice < 1U; ++planeSlice)
    {
        // AdjustPlaneResource

        // Create the texture
        for (uint32_t arraySlice = 0; arraySlice < image_asset_header->array_layers; ++arraySlice)
        {
            size_t w = image_asset_header->width;
            size_t h = image_asset_header->height;
            size_t d = image_asset_header->depth;
            for (uint32_t mipSlice = 0; mipSlice < image_asset_header->mip_levels; ++mipSlice)
            {

                size_t NumBytes = 0U;
                size_t RowBytes = 0U;
                size_t NumRows = 0U;
                if (!GetSurfaceInfo(w, h, dds_format, &NumBytes, &RowBytes, &NumRows))
                {
                    return false;
                }

                if (NumBytes > UINT32_MAX || RowBytes > UINT32_MAX || NumRows > UINT32_MAX)
                {
                    return false;
                }

                // GetLoadFunctionsLoadFunctionsMap libANGLE/renderer/load_functions_table_autogen.cpp
                // LoadToNative
                // LoadCompressedToNative
                size_t inputRowSize = RowBytes;
                size_t inputNumRows = NumRows;
                size_t inputSliceSize = NumBytes;
                size_t inputNumSlices = d;

                // MemcpySubresource d3dx12.h
                uint32_t dstSubresource = brx_load_image_asset_calculate_subresource_index(mipSlice, arraySlice, planeSlice, image_asset_header->mip_levels, image_asset_header->array_layers);
                assert(dstSubresource < subresource_count);

                assert(inputNumSlices == subresource_memcpy_dests[dstSubresource].output_slice_count);
                assert(inputNumRows == subresource_memcpy_dests[dstSubresource].output_row_count);
                assert(inputRowSize == subresource_memcpy_dests[dstSubresource].output_row_size);

                if (inputSliceSize == subresource_memcpy_dests[dstSubresource].output_slice_pitch && inputRowSize == subresource_memcpy_dests[dstSubresource].output_row_pitch)
                {
                    assert(input_stream->seek(0, LOAD_ASSET_INPUT_STREAM_SEEK_CUR) == inputSkipBytes);

                    ptrdiff_t BytesRead = input_stream->read(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(staging_upload_buffer_base) + subresource_memcpy_dests[dstSubresource].staging_upload_buffer_offset), inputSliceSize * inputNumSlices);
                    if (BytesRead == -1 || static_cast<size_t>(BytesRead) < (inputSliceSize * inputNumSlices))
                    {
                        return false;
                    }
                }
                else if (inputRowSize == subresource_memcpy_dests[dstSubresource].output_row_pitch)
                {
                    assert(inputSliceSize <= subresource_memcpy_dests[dstSubresource].output_slice_pitch);

                    for (size_t z = 0; z < inputNumSlices; ++z)
                    {
                        assert(input_stream->seek(0, LOAD_ASSET_INPUT_STREAM_SEEK_CUR) == (inputSkipBytes + inputSliceSize * z));

                        ptrdiff_t BytesRead = input_stream->read(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(staging_upload_buffer_base) + (subresource_memcpy_dests[dstSubresource].staging_upload_buffer_offset + subresource_memcpy_dests[dstSubresource].output_slice_pitch * z)), inputSliceSize);
                        if (BytesRead == -1 || static_cast<size_t>(BytesRead) < inputSliceSize)
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    assert(inputSliceSize <= subresource_memcpy_dests[dstSubresource].output_slice_pitch);
                    assert(inputRowSize <= subresource_memcpy_dests[dstSubresource].output_row_pitch);

                    for (size_t z = 0; z < inputNumSlices; ++z)
                    {
                        for (size_t y = 0; y < inputNumRows; ++y)
                        {
                            assert(input_stream->seek(0, LOAD_ASSET_INPUT_STREAM_SEEK_CUR) == (inputSkipBytes + inputSliceSize * z + inputRowSize * y));

                            ptrdiff_t BytesRead = input_stream->read(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(staging_upload_buffer_base) + (subresource_memcpy_dests[dstSubresource].staging_upload_buffer_offset + subresource_memcpy_dests[dstSubresource].output_slice_pitch * z + subresource_memcpy_dests[dstSubresource].output_row_pitch * y)), inputRowSize);
                            if (BytesRead == -1 || static_cast<size_t>(BytesRead) < inputRowSize)
                            {
                                return false;
                            }
                        }
                    }
                }

                inputSkipBytes += inputSliceSize * inputNumSlices;

                w = w >> 1;
                h = h >> 1;
                d = d >> 1;
                if (w == 0)
                {
                    w = 1;
                }
                if (h == 0)
                {
                    h = 1;
                }
                if (d == 0)
                {
                    d = 1;
                }
            }
        }
    }

    uint8_t u_assert_only[1];
    assert(input_stream->read(u_assert_only, sizeof(uint8_t)) == 0);
    return true;
}

//--------------------------------------------------------------------------------------
static inline DDS_FORMAT GetDDSFormat(DDS_PIXELFORMAT const *ddpf)
{
    if (ddpf->dwFlags & DDPF_RGB)
    {
        // Note that sRGB formats are written using the "DX10" extended header

        switch (ddpf->dwRGBBitCount)
        {
        case 32U:
            if (0X000000FFU == ddpf->dwRBitMask && 0X0000FF00U == ddpf->dwGBitMask && 0X00FF0000U == ddpf->dwBBitMask && 0XFF000000U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_R8G8B8A8_UNORM;
            }

            if (0X00FF0000U == ddpf->dwRBitMask && 0X0000FF00U == ddpf->dwGBitMask && 0X000000FFU == ddpf->dwBBitMask && 0XFF000000U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_B8G8R8A8_UNORM;
            }

            if (0X00FF0000U == ddpf->dwRBitMask && 0X0000FF00U == ddpf->dwGBitMask && 0X000000FFU == ddpf->dwBBitMask && 0X00000000U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_B8G8R8X8_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8

            // Note that many common DDS reader/writers (including D3DX) swap the
            // the RED/BLUE masks for 10:10:10:2 formats. We assume
            // below that the 'backwards' header mask is being used since it is most
            // likely written by D3DX. The more robust solution is to use the 'DX10'
            // header extension and specify the DDS_FORMAT_R10G10B10A2_UNORM format directly

            // For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
            if (0X3FF00000U == ddpf->dwRBitMask && 0X000FFC00U == ddpf->dwGBitMask && 0X000003FFU == ddpf->dwBBitMask && 0XC0000000U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_R10G10B10A2_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10
            if (0X0000FFFFU == ddpf->dwRBitMask && 0XFFFF0000U == ddpf->dwGBitMask && 0X00000000U == ddpf->dwBBitMask && 0X00000000U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_R16G16_UNORM;
            }

            if (0XFFFFFFFFU == ddpf->dwRBitMask && 0X00000000U == ddpf->dwGBitMask && 0X00000000U == ddpf->dwBBitMask && 0X00000000U == ddpf->dwABitMask)
            {
                // Only 32-bit color channel format in D3D9 was R32F
                return DDS_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
            }
            break;

        case 24U:
            // No 24bpp DXGI formats aka D3DFMT_R8G8B8
            break;

        case 16U:
            if (0X7C00U == ddpf->dwRBitMask && 0X03E0U == ddpf->dwGBitMask && 0X001FU == ddpf->dwBBitMask && 0X8000U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_B5G5R5A1_UNORM;
            }
            if (0XF800U == ddpf->dwRBitMask && 0X07E0U == ddpf->dwGBitMask && 0X001FU == ddpf->dwBBitMask && 0X0000U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_B5G6R5_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5

            if (0X0F00U == ddpf->dwRBitMask && 0X00F0U == ddpf->dwGBitMask && 0X000FU == ddpf->dwBBitMask && 0XF000U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_B4G4R4A4_UNORM;
            }

            // NVTT versions 1.x wrote this as RGB instead of LUMINANCE
            if (0X00FFU == ddpf->dwRBitMask && 0X0000U == ddpf->dwGBitMask && 0X0000U == ddpf->dwBBitMask && 0XFF00U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_R8G8_UNORM;
            }
            if (0XFFFFU == ddpf->dwRBitMask && 0X0000U == ddpf->dwGBitMask && 0X0000U == ddpf->dwBBitMask && 0X0000U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_R16_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4

            // No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
            break;

        case 8U:
            // NVTT versions 1.x wrote this as RGB instead of LUMINANCE
            if (0XFFU == ddpf->dwRBitMask && 0X00U == ddpf->dwGBitMask && 0X00U == ddpf->dwBBitMask && 0X00U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_R8_UNORM;
            }

            // No 3:3:2 or paletted DXGI formats aka D3DFMT_R3G3B2, D3DFMT_P8
            break;
        }
    }
    else if (ddpf->dwFlags & DDPF_LUMINANCE)
    {
        switch (ddpf->dwRGBBitCount)
        {
        case 16U:
            if (ddpf->dwRBitMask == 0x0000ffff && ddpf->dwGBitMask == 0x00000000 && ddpf->dwBBitMask == 0x00000000 && ddpf->dwABitMask == 0x00000000)
            {
                return DDS_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
            }

            if (ddpf->dwRBitMask == 0x000000ff && ddpf->dwGBitMask == 0x00000000 && ddpf->dwBBitMask == 0x00000000 && ddpf->dwABitMask == 0x0000ff00)
            {
                return DDS_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
            }
            break;

        case 8U:
            if (0XFFU == ddpf->dwRBitMask && 0x00U == ddpf->dwGBitMask && 0X00U == ddpf->dwBBitMask && 0X00U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
            }

            // No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4

            if (ddpf->dwRBitMask == 0x000000ff && ddpf->dwGBitMask == 0x00000000 && ddpf->dwBBitMask == 0x00000000 && ddpf->dwABitMask == 0x0000ff00)
            {
                return DDS_FORMAT_R8G8_UNORM; // Some DDS writers assume the bitcount should be 8 instead of 16
            }
            break;
        }
    }
    else if (ddpf->dwFlags & DDPF_ALPHA)
    {
        if (8U == ddpf->dwRGBBitCount)
        {
            return DDS_FORMAT_A8_UNORM;
        }
    }
    else if (ddpf->dwFlags & DDPF_BUMPDUDV)
    {
        switch (ddpf->dwRGBBitCount)
        {
        case 32U:
            if (0X000000FFU == ddpf->dwRBitMask && 0X0000FF00U == ddpf->dwGBitMask && 0X00FF0000U == ddpf->dwBBitMask && 0XFF000000U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_R8G8B8A8_SNORM; // D3DX10/11 writes this out as DX10 extension
            }

            if (0X0000FFFFU == ddpf->dwRBitMask && 0XFFFF0000U == ddpf->dwGBitMask && 0X00000000U == ddpf->dwBBitMask && 0X00000000U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_R16G16_SNORM; // D3DX10/11 writes this out as DX10 extension
            }

            // No DXGI format maps to ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000) aka D3DFMT_A2W10V10U10
            break;

        case 16U:
            if (0X00FFU == ddpf->dwRBitMask && 0XFF00U == ddpf->dwGBitMask && 0X0000U == ddpf->dwBBitMask && 0X0000U == ddpf->dwABitMask)
            {
                return DDS_FORMAT_R8G8_SNORM; // D3DX10/11 writes this out as DX10 extension
            }
            break;
        }

        // No DXGI format maps to DDPF_BUMPLUMINANCE aka D3DFMT_L6V5U5, D3DFMT_X8L8V8U8
    }
    else if (ddpf->dwFlags & DDPF_FOURCC)
    {
        if (MakeFourCC('D', 'X', 'T', '1') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_BC1_UNORM;
        }
        if (MakeFourCC('D', 'X', 'T', '3') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_BC2_UNORM;
        }
        if (MakeFourCC('D', 'X', 'T', '5') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_BC3_UNORM;
        }

        // While pre-multiplied alpha isn't directly supported by the DXGI formats,
        // they are basically the same as these BC formats so they can be mapped
        if (MakeFourCC('D', 'X', 'T', '2') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_BC2_UNORM;
        }
        if (MakeFourCC('D', 'X', 'T', '4') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_BC3_UNORM;
        }

        if (MakeFourCC('A', 'T', 'I', '1') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_BC4_UNORM;
        }
        if (MakeFourCC('B', 'C', '4', 'U') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_BC4_UNORM;
        }
        if (MakeFourCC('B', 'C', '4', 'S') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_BC4_SNORM;
        }

        if (MakeFourCC('A', 'T', 'I', '2') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_BC5_UNORM;
        }
        if (MakeFourCC('B', 'C', '5', 'U') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_BC5_UNORM;
        }
        if (MakeFourCC('B', 'C', '5', 'S') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_BC5_SNORM;
        }

        // BC6H and BC7 are written using the "DX10" extended header

        if (MakeFourCC('R', 'G', 'B', 'G') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_R8G8_B8G8_UNORM;
        }
        if (MakeFourCC('G', 'R', 'G', 'B') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_G8R8_G8B8_UNORM;
        }

        if (MakeFourCC('Y', 'U', 'Y', '2') == ddpf->dwFourCC)
        {
            return DDS_FORMAT_YUY2;
        }

        // Check for D3DFORMAT enums being set here
        switch (ddpf->dwFourCC)
        {
        case 36: // D3DFMT_A16B16G16R16
            return DDS_FORMAT_R16G16B16A16_UNORM;

        case 110: // D3DFMT_Q16W16V16U16
            return DDS_FORMAT_R16G16B16A16_SNORM;

        case 111: // D3DFMT_R16F
            return DDS_FORMAT_R16_FLOAT;

        case 112: // D3DFMT_G16R16F
            return DDS_FORMAT_R16G16_FLOAT;

        case 113: // D3DFMT_A16B16G16R16F
            return DDS_FORMAT_R16G16B16A16_FLOAT;

        case 114: // D3DFMT_R32F
            return DDS_FORMAT_R32_FLOAT;

        case 115: // D3DFMT_G32R32F
            return DDS_FORMAT_R32G32_FLOAT;

        case 116: // D3DFMT_A32B32G32R32F
            return DDS_FORMAT_R32G32B32A32_FLOAT;
        }
    }

    return DDS_FORMAT_UNKNOWN;
}

static inline bool GetSurfaceInfo(size_t width, size_t height, uint32_t fmt, size_t *outNumBytes, size_t *outRowBytes, size_t *outNumRows)
{
    uint64_t numBytes = 0;
    uint64_t rowBytes = 0;
    uint64_t numRows = 0;

    bool bc = false;
    bool packed = false;
    bool planar = false;
    size_t bpe = 0;

    switch (fmt)
    {
    case DDS_FORMAT_BC1_TYPELESS:
    case DDS_FORMAT_BC1_UNORM:
    case DDS_FORMAT_BC1_UNORM_SRGB:
    case DDS_FORMAT_BC4_TYPELESS:
    case DDS_FORMAT_BC4_UNORM:
    case DDS_FORMAT_BC4_SNORM:
        bc = true;
        bpe = 8;
        break;

    case DDS_FORMAT_BC2_TYPELESS:
    case DDS_FORMAT_BC2_UNORM:
    case DDS_FORMAT_BC2_UNORM_SRGB:
    case DDS_FORMAT_BC3_TYPELESS:
    case DDS_FORMAT_BC3_UNORM:
    case DDS_FORMAT_BC3_UNORM_SRGB:
    case DDS_FORMAT_BC5_TYPELESS:
    case DDS_FORMAT_BC5_UNORM:
    case DDS_FORMAT_BC5_SNORM:
    case DDS_FORMAT_BC6H_TYPELESS:
    case DDS_FORMAT_BC6H_UF16:
    case DDS_FORMAT_BC6H_SF16:
    case DDS_FORMAT_BC7_TYPELESS:
    case DDS_FORMAT_BC7_UNORM:
    case DDS_FORMAT_BC7_UNORM_SRGB:
        bc = true;
        bpe = 16;
        break;

    case DDS_FORMAT_R8G8_B8G8_UNORM:
    case DDS_FORMAT_G8R8_G8B8_UNORM:
    case DDS_FORMAT_YUY2:
        packed = true;
        bpe = 4;
        break;

    case DDS_FORMAT_Y210:
    case DDS_FORMAT_Y216:
        packed = true;
        bpe = 8;
        break;

    case DDS_FORMAT_NV12:
    case DDS_FORMAT_420_OPAQUE:
    case DDS_FORMAT_P208:
        if (0U != (height % 2U))
        {
            // Requires a height alignment of 2.
            return false;
        }
        planar = true;
        bpe = 2;
        break;

    case DDS_FORMAT_P010:
    case DDS_FORMAT_P016:
        if (0U != (height % 2U))
        {
            // Requires a height alignment of 2.
            return false;
        }
        planar = true;
        bpe = 4;
        break;

    default:
        break;
    }

    if (bc)
    {
        uint64_t numBlocksWide = 0U;
        if (width > 0U)
        {
            numBlocksWide = std::max<uint64_t>(1U, (uint64_t(width) + 3U) / 4U);
        }
        uint64_t numBlocksHigh = 0U;
        if (height > 0U)
        {
            numBlocksHigh = std::max<uint64_t>(1U, (uint64_t(height) + 3U) / 4U);
        }
        rowBytes = numBlocksWide * bpe;
        numRows = numBlocksHigh;
        numBytes = rowBytes * numBlocksHigh;
    }
    else if (packed)
    {
        rowBytes = ((uint64_t(width) + 1U) >> 1U) * bpe;
        numRows = uint64_t(height);
        numBytes = rowBytes * height;
    }
    else if (DDS_FORMAT_NV11 == fmt)
    {
        rowBytes = ((uint64_t(width) + 3U) >> 2U) * 4U;
        numRows = uint64_t(height) * 2U; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
        numBytes = rowBytes * numRows;
    }
    else if (planar)
    {
        rowBytes = ((uint64_t(width) + 1U) >> 1U) * bpe;
        numBytes = (rowBytes * uint64_t(height)) + ((rowBytes * uint64_t(height) + 1U) >> 1U);
        numRows = height + ((uint64_t(height) + 1U) >> 1U);
    }
    else
    {
        size_t bpp = BitsPerPixel(fmt);
        if (0U == bpp)
        {
            return false;
        }
        rowBytes = (uint64_t(width) * bpp + 7U) / 8U; // round up to nearest byte
        numRows = uint64_t(height);
        numBytes = rowBytes * height;
    }

#if defined(_MSC_VER)
    // https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
#if defined(_M_X64) || defined(_M_ARM64)
    static_assert(8U == sizeof(size_t), "Not a 64-bit platform!");
#elif defined(_M_IX86) || defined(_M_ARM)
    static_assert(4U == sizeof(size_t), "Not a 32-bit platform!");
    if (numBytes > UINT32_MAX || rowBytes > UINT32_MAX || numRows > UINT32_MAX)
    {
        return false;
    }
#else
#error Unknown Architecture
#endif
#elif defined(__GNUC__)
    // https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#if defined(__x86_64__) || defined(__aarch64__)
    static_assert(8U == sizeof(size_t), "Not a 64-bit platform!");
#elif defined(__i386__) || defined(__arm__)
    static_assert(4U == sizeof(size_t), "Not a 32-bit platform!");
    if (numBytes > UINT32_MAX || rowBytes > UINT32_MAX || numRows > UINT32_MAX)
    {
        return false;
    }
#else
#error Unknown Architecture
#endif
#else
#error Unknown Compiler
#endif

    if (outNumBytes)
    {
        (*outNumBytes) = static_cast<size_t>(numBytes);
    }
    if (outRowBytes)
    {
        (*outRowBytes) = static_cast<size_t>(rowBytes);
    }
    if (outNumRows)
    {
        (*outNumRows) = static_cast<size_t>(numRows);
    }
    return true;
}

static inline size_t BitsPerPixel(uint32_t fmt)
{
    switch (fmt)
    {
    case DDS_FORMAT_R32G32B32A32_TYPELESS:
    case DDS_FORMAT_R32G32B32A32_FLOAT:
    case DDS_FORMAT_R32G32B32A32_UINT:
    case DDS_FORMAT_R32G32B32A32_SINT:
        return 128;

    case DDS_FORMAT_R32G32B32_TYPELESS:
    case DDS_FORMAT_R32G32B32_FLOAT:
    case DDS_FORMAT_R32G32B32_UINT:
    case DDS_FORMAT_R32G32B32_SINT:
        return 96;

    case DDS_FORMAT_R16G16B16A16_TYPELESS:
    case DDS_FORMAT_R16G16B16A16_FLOAT:
    case DDS_FORMAT_R16G16B16A16_UNORM:
    case DDS_FORMAT_R16G16B16A16_UINT:
    case DDS_FORMAT_R16G16B16A16_SNORM:
    case DDS_FORMAT_R16G16B16A16_SINT:
    case DDS_FORMAT_R32G32_TYPELESS:
    case DDS_FORMAT_R32G32_FLOAT:
    case DDS_FORMAT_R32G32_UINT:
    case DDS_FORMAT_R32G32_SINT:
    case DDS_FORMAT_R32G8X24_TYPELESS:
    case DDS_FORMAT_D32_FLOAT_S8X24_UINT:
    case DDS_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DDS_FORMAT_X32_TYPELESS_G8X24_UINT:
    case DDS_FORMAT_Y416:
    case DDS_FORMAT_Y210:
    case DDS_FORMAT_Y216:
        return 64;

    case DDS_FORMAT_R10G10B10A2_TYPELESS:
    case DDS_FORMAT_R10G10B10A2_UNORM:
    case DDS_FORMAT_R10G10B10A2_UINT:
    case DDS_FORMAT_R11G11B10_FLOAT:
    case DDS_FORMAT_R8G8B8A8_TYPELESS:
    case DDS_FORMAT_R8G8B8A8_UNORM:
    case DDS_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DDS_FORMAT_R8G8B8A8_UINT:
    case DDS_FORMAT_R8G8B8A8_SNORM:
    case DDS_FORMAT_R8G8B8A8_SINT:
    case DDS_FORMAT_R16G16_TYPELESS:
    case DDS_FORMAT_R16G16_FLOAT:
    case DDS_FORMAT_R16G16_UNORM:
    case DDS_FORMAT_R16G16_UINT:
    case DDS_FORMAT_R16G16_SNORM:
    case DDS_FORMAT_R16G16_SINT:
    case DDS_FORMAT_R32_TYPELESS:
    case DDS_FORMAT_D32_FLOAT:
    case DDS_FORMAT_R32_FLOAT:
    case DDS_FORMAT_R32_UINT:
    case DDS_FORMAT_R32_SINT:
    case DDS_FORMAT_R24G8_TYPELESS:
    case DDS_FORMAT_D24_UNORM_S8_UINT:
    case DDS_FORMAT_R24_UNORM_X8_TYPELESS:
    case DDS_FORMAT_X24_TYPELESS_G8_UINT:
    case DDS_FORMAT_R9G9B9E5_SHAREDEXP:
    case DDS_FORMAT_R8G8_B8G8_UNORM:
    case DDS_FORMAT_G8R8_G8B8_UNORM:
    case DDS_FORMAT_B8G8R8A8_UNORM:
    case DDS_FORMAT_B8G8R8X8_UNORM:
    case DDS_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case DDS_FORMAT_B8G8R8A8_TYPELESS:
    case DDS_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DDS_FORMAT_B8G8R8X8_TYPELESS:
    case DDS_FORMAT_B8G8R8X8_UNORM_SRGB:
    case DDS_FORMAT_AYUV:
    case DDS_FORMAT_Y410:
    case DDS_FORMAT_YUY2:
        return 32;

    case DDS_FORMAT_P010:
    case DDS_FORMAT_P016:
    case DDS_FORMAT_V408:
        return 24;

    case DDS_FORMAT_R8G8_TYPELESS:
    case DDS_FORMAT_R8G8_UNORM:
    case DDS_FORMAT_R8G8_UINT:
    case DDS_FORMAT_R8G8_SNORM:
    case DDS_FORMAT_R8G8_SINT:
    case DDS_FORMAT_R16_TYPELESS:
    case DDS_FORMAT_R16_FLOAT:
    case DDS_FORMAT_D16_UNORM:
    case DDS_FORMAT_R16_UNORM:
    case DDS_FORMAT_R16_UINT:
    case DDS_FORMAT_R16_SNORM:
    case DDS_FORMAT_R16_SINT:
    case DDS_FORMAT_B5G6R5_UNORM:
    case DDS_FORMAT_B5G5R5A1_UNORM:
    case DDS_FORMAT_A8P8:
    case DDS_FORMAT_B4G4R4A4_UNORM:
    case DDS_FORMAT_P208:
    case DDS_FORMAT_V208:
        return 16;

    case DDS_FORMAT_NV12:
    case DDS_FORMAT_420_OPAQUE:
    case DDS_FORMAT_NV11:
        return 12;

    case DDS_FORMAT_R8_TYPELESS:
    case DDS_FORMAT_R8_UNORM:
    case DDS_FORMAT_R8_UINT:
    case DDS_FORMAT_R8_SNORM:
    case DDS_FORMAT_R8_SINT:
    case DDS_FORMAT_A8_UNORM:
    case DDS_FORMAT_BC2_TYPELESS:
    case DDS_FORMAT_BC2_UNORM:
    case DDS_FORMAT_BC2_UNORM_SRGB:
    case DDS_FORMAT_BC3_TYPELESS:
    case DDS_FORMAT_BC3_UNORM:
    case DDS_FORMAT_BC3_UNORM_SRGB:
    case DDS_FORMAT_BC5_TYPELESS:
    case DDS_FORMAT_BC5_UNORM:
    case DDS_FORMAT_BC5_SNORM:
    case DDS_FORMAT_BC6H_TYPELESS:
    case DDS_FORMAT_BC6H_UF16:
    case DDS_FORMAT_BC6H_SF16:
    case DDS_FORMAT_BC7_TYPELESS:
    case DDS_FORMAT_BC7_UNORM:
    case DDS_FORMAT_BC7_UNORM_SRGB:
    case DDS_FORMAT_AI44:
    case DDS_FORMAT_IA44:
    case DDS_FORMAT_P8:
        return 8;

    case DDS_FORMAT_R1_UNORM:
        return 1;

    case DDS_FORMAT_BC1_TYPELESS:
    case DDS_FORMAT_BC1_UNORM:
    case DDS_FORMAT_BC1_UNORM_SRGB:
    case DDS_FORMAT_BC4_TYPELESS:
    case DDS_FORMAT_BC4_UNORM:
    case DDS_FORMAT_BC4_SNORM:
        return 4;

    default:
        return 0;
    }
}

static inline uint32_t DDSGetFormatPlaneCount(DDS_FORMAT dds_format)
{
    static uint32_t const dds_format_plane_count_table[] = {
        0, // DDS_FORMAT_UNKNOWN
        1, // DDS_FORMAT_R32G32B32A32_TYPELESS
        1, // DDS_FORMAT_R32G32B32A32_FLOAT
        1, // DDS_FORMAT_R32G32B32A32_UINT
        1, // DDS_FORMAT_R32G32B32A32_SINT
        1, // DDS_FORMAT_R32G32B32_TYPELESS
        1, // DDS_FORMAT_R32G32B32_FLOAT
        1, // DDS_FORMAT_R32G32B32_UINT
        1, // DDS_FORMAT_R32G32B32_SINT
        1, // DDS_FORMAT_R16G16B16A16_TYPELESS
        1, // DDS_FORMAT_R16G16B16A16_FLOAT
        1, // DDS_FORMAT_R16G16B16A16_UNORM
        1, // DDS_FORMAT_R16G16B16A16_UINT
        1, // DDS_FORMAT_R16G16B16A16_SNORM
        1, // DDS_FORMAT_R16G16B16A16_SINT
        1, // DDS_FORMAT_R32G32_TYPELESS
        1, // DDS_FORMAT_R32G32_FLOAT
        1, // DDS_FORMAT_R32G32_UINT
        1, // DDS_FORMAT_R32G32_SINT
        2, // DDS_FORMAT_R32G8X24_TYPELESS
        2, // DDS_FORMAT_D32_FLOAT_S8X24_UINT
        2, // DDS_FORMAT_R32_FLOAT_X8X24_TYPELESS
        2, // DDS_FORMAT_X32_TYPELESS_G8X24_UINT
        1, // DDS_FORMAT_R10G10B10A2_TYPELESS
        1, // DDS_FORMAT_R10G10B10A2_UNORM
        1, // DDS_FORMAT_R10G10B10A2_UINT
        1, // DDS_FORMAT_R11G11B10_FLOAT
        1, // DDS_FORMAT_R8G8B8A8_TYPELESS
        1, // DDS_FORMAT_R8G8B8A8_UNORM
        1, // DDS_FORMAT_R8G8B8A8_UNORM_SRGB
        1, // DDS_FORMAT_R8G8B8A8_UINT
        1, // DDS_FORMAT_R8G8B8A8_SNORM
        1, // DDS_FORMAT_R8G8B8A8_SINT
        1, // DDS_FORMAT_R16G16_TYPELESS
        1, // DDS_FORMAT_R16G16_FLOAT
        1, // DDS_FORMAT_R16G16_UNORM
        1, // DDS_FORMAT_R16G16_UINT
        1, // DDS_FORMAT_R16G16_SNORM
        1, // DDS_FORMAT_R16G16_SINT
        1, // DDS_FORMAT_R32_TYPELESS
        1, // DDS_FORMAT_D32_FLOAT
        1, // DDS_FORMAT_R32_FLOAT
        1, // DDS_FORMAT_R32_UINT
        1, // DDS_FORMAT_R32_SINT
        2, // DDS_FORMAT_R24G8_TYPELESS
        2, // DDS_FORMAT_D24_UNORM_S8_UINT
        2, // DDS_FORMAT_R24_UNORM_X8_TYPELESS
        2, // DDS_FORMAT_X24_TYPELESS_G8_UINT
        1, // DDS_FORMAT_R8G8_TYPELESS
        1, // DDS_FORMAT_R8G8_UNORM
        1, // DDS_FORMAT_R8G8_UINT
        1, // DDS_FORMAT_R8G8_SNORM
        1, // DDS_FORMAT_R8G8_SINT
        1, // DDS_FORMAT_R16_TYPELESS
        1, // DDS_FORMAT_R16_FLOAT
        1, // DDS_FORMAT_D16_UNORM
        1, // DDS_FORMAT_R16_UNORM
        1, // DDS_FORMAT_R16_UINT
        1, // DDS_FORMAT_R16_SNORM
        1, // DDS_FORMAT_R16_SINT
        1, // DDS_FORMAT_R8_TYPELESS
        1, // DDS_FORMAT_R8_UNORM
        1, // DDS_FORMAT_R8_UINT
        1, // DDS_FORMAT_R8_SNORM
        1, // DDS_FORMAT_R8_SINT
        1, // DDS_FORMAT_A8_UNORM
        1, // DDS_FORMAT_R1_UNORM
        1, // DDS_FORMAT_R9G9B9E5_SHAREDEXP
        1, // DDS_FORMAT_R8G8_B8G8_UNORM
        1, // DDS_FORMAT_G8R8_G8B8_UNORM
        1, // DDS_FORMAT_BC1_TYPELESS
        1, // DDS_FORMAT_BC1_UNORM
        1, // DDS_FORMAT_BC1_UNORM_SRGB
        1, // DDS_FORMAT_BC2_TYPELESS
        1, // DDS_FORMAT_BC2_UNORM
        1, // DDS_FORMAT_BC2_UNORM_SRGB
        1, // DDS_FORMAT_BC3_TYPELESS
        1, // DDS_FORMAT_BC3_UNORM
        1, // DDS_FORMAT_BC3_UNORM_SRGB
        1, // DDS_FORMAT_BC4_TYPELESS
        1, // DDS_FORMAT_BC4_UNORM
        1, // DDS_FORMAT_BC4_SNORM
        1, // DDS_FORMAT_BC5_TYPELESS
        1, // DDS_FORMAT_BC5_UNORM
        1, // DDS_FORMAT_BC5_SNORM
        1, // DDS_FORMAT_B5G6R5_UNORM
        1, // DDS_FORMAT_B5G5R5A1_UNORM
        1, // DDS_FORMAT_B8G8R8A8_UNORM
        1, // DDS_FORMAT_B8G8R8X8_UNORM
        1, // DDS_FORMAT_R10G10B10_XR_BIAS_A2_UNORM
        1, // DDS_FORMAT_B8G8R8A8_TYPELESS
        1, // DDS_FORMAT_B8G8R8A8_UNORM_SRGB
        1, // DDS_FORMAT_B8G8R8X8_TYPELESS
        1, // DDS_FORMAT_B8G8R8X8_UNORM_SRGB
        1, // DDS_FORMAT_BC6H_TYPELESS
        1, // DDS_FORMAT_BC6H_UF16
        1, // DDS_FORMAT_BC6H_SF16
        1, // DDS_FORMAT_BC7_TYPELESS
        1, // DDS_FORMAT_BC7_UNORM
        1, // DDS_FORMAT_BC7_UNORM_SRGB
        1, // DDS_FORMAT_AYUV
        1, // DDS_FORMAT_Y410
        1, // DDS_FORMAT_Y416
        2, // DDS_FORMAT_NV12
        2, // DDS_FORMAT_P010
        2, // DDS_FORMAT_P016
        1, // DDS_FORMAT_420_OPAQUE
        1, // DDS_FORMAT_YUY2
        1, // DDS_FORMAT_Y210
        1, // DDS_FORMAT_Y216
        2, // DDS_FORMAT_NV11
        1, // DDS_FORMAT_AI44
        1, // DDS_FORMAT_IA44
        1, // DDS_FORMAT_P8
        1, // DDS_FORMAT_A8P8
        1, // DDS_FORMAT_B4G4R4A4_UNORM //115
        0, // DDS_FORMAT_116
        0, // DDS_FORMAT_117
        0, // DDS_FORMAT_118
        0, // DDS_FORMAT_119
        0, // DDS_FORMAT_120
        0, // DDS_FORMAT_121
        0, // DDS_FORMAT_122
        0, // DDS_FORMAT_123
        0, // DDS_FORMAT_124
        0, // DDS_FORMAT_125
        0, // DDS_FORMAT_126
        0, // DDS_FORMAT_127
        0, // DDS_FORMAT_128
        0, // DDS_FORMAT_129
        0, // DDS_FORMAT_P208 //130
        0, // DDS_FORMAT_V208
        0  // DDS_FORMAT_V408
    };

    return dds_format_plane_count_table[dds_format];
}

static inline bool IsDepthStencil(DDS_FORMAT dds_format)
{
    switch (dds_format)
    {
    case DDS_FORMAT_R32G8X24_TYPELESS:
    case DDS_FORMAT_D32_FLOAT_S8X24_UINT:
    case DDS_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DDS_FORMAT_X32_TYPELESS_G8X24_UINT:
    case DDS_FORMAT_D32_FLOAT:
    case DDS_FORMAT_R24G8_TYPELESS:
    case DDS_FORMAT_D24_UNORM_S8_UINT:
    case DDS_FORMAT_R24_UNORM_X8_TYPELESS:
    case DDS_FORMAT_X24_TYPELESS_G8_UINT:
    case DDS_FORMAT_D16_UNORM:
        return true;
    default:
        return false;
    }
}
